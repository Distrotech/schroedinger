#!/usr/bin/python

import re
import sys



def error (message):
  print "ERROR: " + message
  exit (-1)



def contains_reserverd_block_header (string):
  if re.search ("^(?:versions|uniforms|defines|else|" \
      "(?:shader|func|if|elif)[ ]+[^:]+):$", string) is not None:
    return True

  return False



def compare_uniform_bind_signatures (a, b):
  if a.endswith ("texture)") is b.endswith ("texture)"):
    return cmp (a, b)

  if a.endswith ("texture)"):
    return -1

  return 1



def compare_uniforms_float (a, b):
  return cmp (a.print_glsl ("float"), b.print_glsl ("float"))



def compare_uniforms_integer (a, b):
  return cmp (a.print_glsl ("integer"), b.print_glsl ("integer"))



class Item:
  def __init__ (self):
    self.version = ".*"

  def matches_version (self, version):
    return re.search ("^" + self.version + "$", version) is not None



class Line (Item):
  def __init__ (self, number, string):
    Item.__init__(self)

    self.number = number
    self.level = 0
    self.content = ""

    match = re.search ("^([ ]*)([^ ].*)$", string)

    if match is not None:
      if len (match.group (1)) % 2 != 0:
        error ("invalid indentation in line " + repr (number) + " '" + string
            + "'")

      if re.search ("^(?:versions|uniforms|defines|else|"
          "(?:shader|func|if|elif)[ ]+[^:]+)$", string) is not None:
        error ("expecting reserved block header '" + match.group (2)
            + "' to be ended with a ':' at line " + repr (number))

      self.level = len (match.group (1)) / 2
      self.content = match.group (2)

  def __repr__ (self):
    string = ""
    level = self.level

    while level > 0:
      string += "  "
      level = level - 1

    string += "(Line: " + repr (self.number) + ", " + repr (self.level) + ", " \
        + repr (self.content) + ", " + repr (self.version) + ")"

    return string

  def decrease_level (self):
    self.level = self.level - 1



class Block (Item):
  def __init__ (self):
    Item.__init__(self)

    self.header = None
    self.children = []

  def __repr__ (self):
    string = ""
    level = self.header.level

    while level > 0:
      string += "  "
      level = level - 1

    string += "(Block: " + repr (self.version) + " " \
        + repr (self.header).lstrip () + "\n"

    for child in self.children:
      string += repr (child) + "\n"

    return string.rstrip ("\n") + ")"

  def parse (self, lines):
    if len (lines) < 2 or (not lines[0].content.endswith (":") and
        not lines[1].level > lines[0].level):
      return

    if lines[1].level <= lines[0].level:
      error ("expecting at least one line in the block '" + lines[0].content
          + "' started at line " + repr (lines[0].number))

    if lines[1].level > lines[0].level + 1:
      error ("invalid indentation in line " + repr (lines[1].number)
          + ", expecting this line to be intended one level further than "
          "the block '" + lines[0].content + "' started at line "
          + repr (lines[0].number))

    if lines[1].level == lines[0].level + 1:
      if not lines[0].content.endswith (":"):
        error ("either invalid indentation in line " + repr (lines[1].number)
            + " or expecting ':' at end of line " + repr (lines[0].number)
            + " to start a block but found '" + lines[0].content + "'")

    self.header = lines[0]
    lines = lines[1:]

    while len (lines) > 0 and lines[0].level >= self.header.level + 1:
      block = Block ()
      result = block.parse (lines)

      if result is None:
        self.children += [lines[0]]
        lines = lines[1:]
      else:
        self.children += [block]
        lines = result

    return lines

  def decrease_level (self):
    self.header.decrease_level ()

    for child in self.children:
      child.decrease_level ()



class Uniform:
  def __init__ (self, shader_index, type, name, read_define, read_slot,
      is_dependent):
    if read_slot > 9:
      error ("read slot for " + repr (name) + " is out of bounds in shader "
          + repr (shader_index) + ". expecting a value in [0..9] but found "
          + repr (read_slot))

    self.shader_index = shader_index
    self.type = type
    self.name = name
    self.read_define = read_define
    self.read_slot = read_slot
    self.is_dependent = is_dependent
    self.glsl_mappings = {
        "integer" : {
            "float"    : "float",
            "vec2"     : "vec2",
            "vec4"     : "vec4",
            "var_u8"   : "uint",
            "var_s16"  : "int",
            "var2_u8"  : "uvec2",
            "var2_s16" : "ivec2",
            "var4_u8"  : "uvec4",
            "var4_s16" : "ivec4"},
        "float"   : {
            "float"    : "float",
            "vec2"     : "vec2",
            "vec4"     : "vec4",
            "var_u8"   : "float",
            "var_s16"  : "float",
            "var2_u8"  : "vec2",
            "var2_s16" : "vec2",
            "var4_u8"  : "vec4",
            "var4_s16" : "vec4"}}
    self.mode_mappings = {
        "integer" : "_INTEGER",
        "float"   :   ""}
    self.signature_mappings = {
        "sampler"  : "GLuint texture",
        "float"    : "float v0",
        "vec2"     : "float v0, float v1",
        "vec4"     : "float v0, float v1, float v2, float v3",
        "var_u8"   : "uint8_t v0",
        "var_s16"  : "int16_t v0",
        "var2_u8"  : "uint8_t v0, uint8_t v1",
        "var2_s16" : "int16_t v0, int16_t v1",
        "var4_u8"  : "uint8_t v0, uint8_t v1, uint8_t v2, uint8_t v3",
        "var4_s16" : "int16_t v0, int16_t v1, int16_t v2, int16_t v3"}
    self.bind_mappings = {
        "integer" : {
            "sampler"  : "glUniform1iARB",
            "float"    : "glUniform1fARB",
            "vec2"     : "glUniform2fARB",
            "vec4"     : "glUniform4fARB",
            "var_u8"   : "glUniform1uiEXT",
            "var_s16"  : "glUniform1iARB",
            "var2_u8"  : "glUniform2uiEXT",
            "var2_s16" : "glUniform2iARB",
            "var4_u8"  : "glUniform4uiEXT",
            "var4_s16" : "glUniform4iARB"},
        "float"   : {
            "sampler"  : "glUniform1iARB",
            "float"    : "glUniform1fARB",
            "vec2"     : "glUniform2fARB",
            "vec4"     : "glUniform4fARB",
            "var_u8"   : "glUniform1fARB",
            "var_s16"  : "glUniform1fARB",
            "var2_u8"  : "glUniform2fARB",
            "var2_s16" : "glUniform2fARB",
            "var4_u8"  : "glUniform4fARB",
            "var4_s16" : "glUniform4fARB"}}
    self.parameter_mappings = {
        "sampler"  : "texture",
        "float"    : "v0",
        "vec2"     : "v0, v1",
        "vec4"     : "v0, v1, v2, v3",
        "var_u8"   : "v0",
        "var_s16"  : "v0",
        "var2_u8"  : "v0, v1",
        "var2_s16" : "v0, v1",
        "var4_u8"  : "v0, v1, v2, v3",
        "var4_s16" : "v0, v1, v2, v3"}

  def print_glsl (self, mode):
    if self.is_dependent:
      return ""

    if mode not in self.glsl_mappings.keys ():
      error ("internal mode error")

    if len (self.read_define) > 0:
      return "      " + self.read_define + self.mode_mappings[mode] \
          + " (\"" + self.name + "\")\n"
    else:
      return "      \"uniform " + self.glsl_mappings[mode][self.type] + " " \
          + self.name + ";\\n\"\n"

  def print_bind_signature (self):
    return "schro_opengl_shader_bind_" + self.name \
        + " (SchroOpenGLShader* shader, " + self.signature_mappings[self.type] \
        + ")"

  def print_bind_case (self, mode):
    bind_mapping = self.bind_mappings[mode][self.type]
    parameter_mapping = self.parameter_mappings[self.type]

    if parameter_mapping == "texture":
      if self.read_slot > 0:
        return \
            "      glActiveTextureARB (GL_TEXTURE0_ARB + " \
            + repr (self.read_slot) + ");\n" \
            "      glBindTexture (GL_TEXTURE_RECTANGLE_ARB, " \
            + parameter_mapping + ");\n" \
            "      " + bind_mapping + " (shader->uniforms->" + self.name \
            + ", " + repr (self.read_slot) + ");\n" \
            "      glActiveTextureARB (GL_TEXTURE0_ARB);\n"
      else:
        return \
            "      glBindTexture (GL_TEXTURE_RECTANGLE_ARB, " \
            + parameter_mapping + ");\n" \
            "      " + bind_mapping + " (shader->uniforms->" + self.name \
            + ", " + repr (self.read_slot) + ");\n"
    else:
      return \
          "      " + bind_mapping + " (shader->uniforms->" + self.name \
          + ", " + parameter_mapping + ");\n"



class Shader:
  def __init__ (self, bunch, version, index):
    self.bunch = bunch
    self.version = version
    self.index = index
    self.defines = []
    self.uniforms = {} # keyed by uniform name
    self.functions = []
    self.used_builtins = []
    self.available_builtins = {
        "write_u8"            : "SHADER_WRITE_U8",
        "write_u8_raw"        : "SHADER_WRITE_U8_RAW",
        "write_vec4_u8"       : "SHADER_WRITE_VEC4_U8",
        "write_vec4_u8_raw"   : "SHADER_WRITE_VEC4_U8_RAW",
        "write_s16"           : "SHADER_WRITE_S16",
        "write_s16_raw"       : "SHADER_WRITE_S16_RAW",
        "write_vec4_s16"      : "SHADER_WRITE_VEC4_S16",
        "write_vec4_s16_raw"  : "SHADER_WRITE_VEC4_S16_RAW",
        "cast_s16_u8"         : "SHADER_CAST_S16_U8",
        "cast_u8_s16"         : "SHADER_CAST_U8_S16",
        "convert_raw_u8"      : "SHADER_CONVERT_RAW_U8",
        "convert_raw_s16"     : "SHADER_CONVERT_RAW_S16",
        "divide_s16"          : "SHADER_DIVIDE_S16",
        "crossfoot2_s16"       : "SHADER_CROSSFOOT2_S16",
        "crossfoot4_s16"       : "SHADER_CROSSFOOT4_S16",
        "ref_weighting_s16"   : "SHADER_REF_WEIGHTING_S16",
        "biref_weighting_s16" : "SHADER_BIREF_WEIGHTING_S16"}
    self.builtin_dependencies = {
        "ref_weighting_s16"   : "divide_s16",
        "biref_weighting_s16" : "divide_s16"}
    self.builtin_uniforms = {
        "ref_weighting_s16"   : [
            ("var_s16", "ref_weight"),
            ("var_s16", "ref_addend"),
            ("var_s16", "ref_divisor")],
        "biref_weighting_s16" : [
            ("var_s16", "ref_weight1"),
            ("var_s16", "ref_weight2"),
            ("var_s16", "ref_addend"),
            ("var_s16", "ref_divisor")]}
    self.glsl_var_mappings = {
        "integer" : {
          "var_u8"   : "uint",
          "var_s16"  : "int",
          "var2_u8"  : "uvec2",
          "var2_s16" : "ivec2",
          "var4_u8"  : "uvec4",
          "var4_s16" : "ivec4"},
        "float"   : {
          "var_u8"   : "float",
          "var_s16"  : "float",
          "var2_u8"  : "vec2",
          "var2_s16" : "vec2",
          "var4_u8"  : "vec4",
          "var4_s16" : "vec4"}}
    self.next_read_slot = 0

  def add_uniform (self, uniform):
    if uniform.name in self.uniforms.keys ():
      if self.uniforms[uniform.name].type != uniform.type:
        error ("uniform name '" + repr (uniform.name)
            + "' within a shader must be unique")
    else:
      self.uniforms[uniform.name] = uniform

  def parse (self, children):
    # parse defines/uniforms/functions
    while len (children) > 0:
      if not isinstance (children[0], Block):
        error ("expecting a block")

      if children[0].header.content == "defines:":
        for child in children[0].children:
          if not isinstance (child, Line):
            error ("expecting lines in the defines block")

          self.parse_define (child)
      elif children[0].header.content == "uniforms:":
        for child in children[0].children:
          if not isinstance (child, Line):
            error ("expecting lines in the uniforms block")

          self.parse_uniform (child)
      elif children[0].header.content.startswith("func "):
        self.parse_builtins (children[0])
        self.parse_read_calls (children[0])

        self.functions += [children[0]]
      else:
        error ("expection defines/uniforms/function block but "
            "found '" + children[0].header.content + "'")

      children = children[1:]

    # check for builtin dependencies
    self.used_builtins.sort ()

    used_builtins = []

    for builtin in self.used_builtins:
      if builtin in self.builtin_dependencies.keys ():
        builtin_dependency = self.builtin_dependencies[builtin]

        if builtin_dependency not in used_builtins:
          used_builtins += [builtin_dependency]

      if builtin not in used_builtins:
        used_builtins += [builtin]

    self.used_builtins = used_builtins

    for builtin in self.used_builtins:
      if builtin in self.builtin_uniforms.keys ():
        for uniform in self.builtin_uniforms[builtin]:
          self.add_uniform (Uniform (self.index, uniform[0], uniform[1], "",
              -1, True))

  def parse_define (self, line):
    if line.matches_version (self.version):
      match = re.search ("^[A-Z0-9]+[ ]+.+$", line.content)

      if match is None:
        error ("expecting define '[A-Z0-9]+[ ]+.+' but found '" \
            + line.content + "' at line " + repr (line.number))

      self.defines += [line]

  def parse_uniform (self, line):
    if line.matches_version (self.version):
      match = re.search ("^([a-z1-9_]+)[ ]+([a-z0-9_]+)$", line.content)

      if match is None:
        error ("expecting uniform '[a-z1-9_]+[ ]+[a-z0-9_]+' but found '"
            + line.content + "' at line " + repr (line.number))

      self.add_uniform (Uniform (self.index, match.group (1), match.group (2),
          "", -1, False))

  def parse_builtins (self, block):
    for child in block.children:
      if isinstance (child, Line):
        if child.matches_version (self.version):
          for builtin in self.available_builtins.keys ():
            if re.search ("([^a-zA-Z0-9_]|^)" + builtin + "([^a-zA-Z0-9_]|$)", \
                child.content) is not None and \
                builtin not in self.used_builtins:
              self.used_builtins += [builtin]
      elif isinstance (child, Block):
        if child.matches_version (self.version):
          self.parse_builtins (child)
      else:
        error ("internal type error")

  def parse_read_calls (self, block):
    for child in block.children:
      if isinstance (child, Line):
        if child.matches_version (self.version):
          string = child.content
          read_call_re = re.compile ("(?:[^a-zA-Z0-9_]|^)"
              "read_([a-z0-9_]+?)(_vec4|)_(u8|s16)(_raw|)(?:[^a-zA-Z0-9_]|$)")
          match = read_call_re.search (string)

          while match is not None:
            read_define = "SHADER_READ"

            if match.group (2) == "_vec4":
              read_define += "_VEC4"

            if match.group (3) == "u8":
              read_define += "_U8"
            elif match.group (3) == "s16":
              read_define += "_S16"
            else:
              error ("internal match error")

            if match.group (4) == "_raw":
              read_define += "_RAW"

            self.add_uniform (Uniform (self.index, "sampler", match.group (1),
                read_define, self.next_read_slot, False))

            self.next_read_slot += 1

            string = string[match.span ()[1]:]
            match = read_call_re.search (string)

          string = child.content
          copy_call_re = re.compile ("(?:[^a-zA-Z0-9_]|^)"
              "copy_([a-z0-9_]+)_(u8|s16)(?:[^a-zA-Z0-9_]|$)")
          match = copy_call_re.search (string)

          while match is not None:
            read_define = "SHADER_COPY"

            if match.group (2) == "u8":
              read_define += "_U8"
            elif match.group (2) == "s16":
              read_define += "_S16"
            else:
              error ("internal match error")

            self.add_uniform (Uniform (self.index, "sampler", match.group (1),
                read_define, self.next_read_slot, False))

            self.next_read_slot += 1

            string = string[match.span ()[1]:]
            match = copy_call_re.search (string)
      elif isinstance (child, Block):
        if child.matches_version (self.version):
          self.parse_read_calls (child)
      else:
        error ("internal type error")

  def replace_vars_with_glsl (self, string, type):
    if type in self.glsl_var_mappings.keys ():
      for key in self.glsl_var_mappings[type].keys ():
        string = re.sub ("([^a-zA-Z0-9_]|^)" + key + " ",
            "\\1" + self.glsl_var_mappings[type][key] + " ", string)
    else:
      error ("internal type error")

    return string

  def replace_numbers (self, string, type):
    if type == "float":
      string = re.sub ("((?:[+\\-*/=,( ]|^)[0-9]+)([^0-9u\\.]|$)", "\\1.0\\2",
          string)

    return string

  def print_glsl (self):
    string = "  { " + self.index + ",\n      \"" + self.bunch + "/" \
        + self.version + "\",\n"
    string += self.print_glsl_mode ("float")
    string = string.rstrip("\n") + ",\n"
    string += self.print_glsl_mode ("integer")

    return string.rstrip(" \n") + " },\n"

  def print_glsl_mode (self, mode):
    postfix = ""
    compare_uniforms = compare_uniforms_float

    if mode == "integer":
      postfix = "_INTEGER"
      compare_uniforms = compare_uniforms_integer

    string = "      SHADER_HEADER" + postfix + "\n"

    # builtins
    for builtin in self.used_builtins:
      string += "      " + self.available_builtins[builtin] + postfix + "\n"

    # defines
    for define in self.defines:
      string += "      \"#define " + define.content + "\\n\"\n"

    # uniforms
    uniforms = self.uniforms.values ()[:]

    uniforms.sort(compare_uniforms)

    for uniform in uniforms:
      string += uniform.print_glsl (mode)

    # functions
    for function in self.functions:
      string += self.print_function (function, mode)

    return string

  def print_function (self, function, type):
    match = re.search ("^func (.*):$", function.header.content)

    if match is None:
      error ("expecting function signature 'func .*:' but found '"
          + function.header.content + "'")

    string = "      \"" + self.replace_vars_with_glsl (match.group (1), type) \
        + " {\\n\"\n"

    for child in function.children:
      if isinstance (child, Line):
        if child.matches_version (self.version):
          string += self.print_function_line (child, type, "  ")
      elif isinstance (child, Block):
        if child.matches_version (self.version):
          result = self.print_function_block (child, type, "  ")

          if result.startswith ("else "):
            string = string.rstrip ("\"\n\\n") + " " + result
          else:
            string += result
      else:
        error ("internal type error")

    return string + "      \"}\\n\"\n"

  def print_function_line (self, line, type, indent):
    string = "      \"" + indent \
         + self.replace_vars_with_glsl (line.content, type) + ";\\n\"\n"

    return self.replace_numbers (string, type)

  def print_function_block (self, block, type, indent):
    match = re.search ("^(if|elif|else)[ ]*(.*):$", block.header.content)

    if match is None:
      error ("expecting function signature '(if|elif|else) .*:' but found '"
          + block.header.content + "'")

    string = ""

    if match.group (1) == "if":
      string += "      \"" + indent + "if (" + match.group (2) + ") {\\n\"\n"
    elif match.group (1) == "elif":
      string += "else if (" + match.group (2) + ") {\\n\"\n"
    elif match.group (1) == "else":
      string += "else {\\n\"\n"
    else:
      error ("internal match error")

    string = self.replace_numbers (string, type)

    for child in block.children:
      if isinstance (child, Line):
        if child.matches_version (self.version):
          string += self.print_function_line (child, type, indent + "  ")
      elif isinstance (child, Block):
        if child.matches_version (self.version):
          result = self.print_function_block (child, type, indent + "  ")

          if result.startswith ("else "):
            string = string.rstrip ("\"\n\\n") + " " + result
          else:
            string += result
      else:
        error ("internal type error")

    return string + "      \"" + indent + "}\\n\"\n"



class ShaderBunch:
  def __init__ (self):
    self.versions = []
    self.indices = {} # keyed by version
    self.shaders = []

  def parse (self, block):
    match = re.search ("^shader[ ]+([a-z0-9_]+):$", block.header.content)

    if match is None:
      error ("expecting header 'shader[ ]+[a-z0-9_]+:' but found '" \
          + block.header.content + "' at line " + repr (block.header.number))

    bunch = match.group (1)
    children = block.children[:]

    # parse versions
    if not isinstance (children[0], Block):
      error ("expecting a 'versions:' block")

    if children[0].header.content != "versions:":
      error ("expecting 'versions:' as first block after " \
         "'shader[ ]+[a-z0-9_]+:' but found '" + children[0].header.content \
         + "' at line " + repr (children[0].header.number))

    for child in children[0].children:
      if not isinstance (child, Line):
        error ("expecting line in 'versions:' block")

      self.parse_version (child)

    children = self.transform_versions (children[1:])

    for version in self.versions:
      shader = Shader (bunch, version, self.indices[version])

      shader.parse (children[:])

      self.shaders += [shader]

  def parse_version (self, line):
    match = re.search ("^([a-z0-9_]+)[ ]+([a-zA-Z0-9_]+)$", line.content)

    if match is None:
      error ("expecting version '[a-z0-9_]+[ ]+[a-zA-Z0-9_]+' but found '" \
          + line.content + "' at line " + repr (line.number))

    version = match.group (1)
    index = match.group (2)

    if contains_reserverd_block_header (version):
      error ("version " + repr (version) + " is a reserved block header")

    if version in self.versions:
      error ("version " + repr (version) + " already defined")

    self.versions += [version]
    self.indices[version] = index

  def transform_versions (self, children):
    transformed = []

    for child in children:
      if isinstance (child, Line):
        transformed += [child]
      elif isinstance (child, Block):
        if contains_reserverd_block_header (child.header.content):
          child.children = self.transform_versions (child.children)
          transformed += [child]
        else:
          header = child.header.content[:-1].strip ()

          if contains_reserverd_block_header (header):
              child.children = self.transform_versions (child.children)
              transformed += [child]
          else:
            version_re = re.compile ("^" + header + "$")

            for version in self.versions:
              if version_re.search (version) is not None:
                break
            else:
              error ("version " + repr (header) + " in line " \
                  + repr (child.header.number)
                  + " doesn't match any defined version in "
                  + repr (self.versions))

            for subchild in child.children:
              if subchild.version != ".*":
                error ("nesting versions is not allowed at line " \
                    + repr (child.number))

              subchild.decrease_level ()
              subchild.version = header

            transformed += self.transform_versions (child.children)
      else:
        error ("internal type error")

    return transformed

  def print_indices (self):
    indices = []

    for shader in self.shaders:
      indices += [shader.index]

    return indices

  def print_glsl (self):
    glsl = ""

    for shader in self.shaders:
      glsl += shader.print_glsl ()

    return glsl



if __name__ == "__main__":
  # parse raw lines
  raw_lines = []

  for string in file ("schroopenglshadercompiler.input", "rb").readlines ():
    if '#' in string:
      string = string[0:string.index ('#')]

    string = string.rstrip ().replace("\t", " ")
    string = re.sub ("[ ]+:", ":", string)

    if len (raw_lines) > 0 and raw_lines[-1].endswith("\\"):
      raw_lines[-1] = raw_lines[-1][:-1].rstrip () + " " + string.lstrip ()
      string = ""

    raw_lines += [string]

  # parse lines from raw lines
  lines = []
  number = 1

  for string in raw_lines:
    if len (string) > 0:
      lines += [Line (number, string)]

    number += 1

  # parse blocks from lines
  blocks = []

  while len (lines) > 0:
    block = Block ()
    lines = block.parse (lines)

    if lines is None:
      error ("somethings wrong with the block structure")

    blocks += [block]

  # parse shaders from blocks
  shader_bunches = []

  for block in blocks:
    shader_bunch = ShaderBunch ()

    shader_bunch.parse (block)

    shader_bunches += [shader_bunch]

  # output header
  output_h = "/* WARNING! Generated header, do not edit! */\n" \
      + file ("schroopenglshadercode.h.template", "rb").read ()

  output_h_index_defines = ""
  count = 0

  for shader_bunch in shader_bunches:
    for index in shader_bunch.print_indices ():
      output_h_index_defines += "#define " + index + " " + repr (count) + "\n"
      count += 1

  output_h_index_defines += "\n#define SCHRO_OPENGL_SHADER_COUNT " \
      + repr (count) + "\n"

  output_h = output_h.replace ("===== SHADER INDEX DEFINES =====\n",
      output_h_index_defines)

  uniform_types = {} # keyed by uniform.name
  uniform_names = []
  uniform_bind_signatures = []
  uniforms = {} # keyed by uniform_bind_signature

  for shader_bunch in shader_bunches:
    for shader in shader_bunch.shaders:
      for uniform in shader.uniforms.values ():
        if uniform.name in uniform_types.keys ():
          if uniform_types[uniform.name] != uniform.type:
            error ("expecting uniform type be consistent per uniform name, "
                "but found '" + uniform_types[uniform.name] + " "
                + uniform.name + "' and '" + uniform.type + " "
                + uniform.name + "'")
        else:
          uniform_types[uniform.name] = uniform.type

        if uniform.name not in uniform_names:
          uniform_names += [uniform.name]

        uniform_bind_signature = uniform.print_bind_signature ()

        if uniform_bind_signature not in uniform_bind_signatures:
          uniform_bind_signatures += [uniform_bind_signature]
          uniforms[uniform_bind_signature] = [uniform]
        else:
          uniforms[uniform_bind_signature] += [uniform]

  uniform_names.sort ()
  uniform_bind_signatures.sort (compare_uniform_bind_signatures)

  output_h_uniform_variables = ""

  for uniform_name in uniform_names:
    output_h_uniform_variables += "  GLint " + uniform_name + ";\n"

  output_h = output_h.replace ("===== UNIFORM VARIABLES =====\n",
      output_h_uniform_variables)

  output_h_uniform_bind_signatures = ""

  for uniform_bind_signature in uniform_bind_signatures:
    output_h_uniform_bind_signatures += "void " + uniform_bind_signature + ";\n"

  output_h = output_h.replace ("===== UNIFORM BIND SIGNATURES =====\n",
      output_h_uniform_bind_signatures)

  file ("schroopenglshadercode.h", "wb").write (output_h)

  # output code
  output_c = "/* WARNING! Generated code, do not edit! */\n" \
      + file ("schroopenglshadercode.c.template", "rb").read ()

  output_c_shader_code = ""

  for shader_bunch in shader_bunches:
    output_c_shader_code += shader_bunch.print_glsl ()

  output_c = output_c.replace ("===== SHADER CODE =====\n",
      output_c_shader_code)

  output_c_uniform_resolvers = ""

  for uniform_name in uniform_names:
    output_c_uniform_resolvers += "  GET_UNIFORM_LOCATION (" + uniform_name \
        + ");\n"

  output_c = output_c.replace ("===== UNIFORM RESOLVERS =====\n",
      output_c_uniform_resolvers)

  output_c_uniform_bind_functions = ""

  for uniform_bind_signature in uniform_bind_signatures:
    output_c_uniform_bind = {"integer" : "", "float" : ""}

    for mode in output_c_uniform_bind.keys ():
      uniform_bind_cases = []
      uniform_bind_case_shader_indices = {} # keyed by uniform_bind_case

      for uniform in uniforms[uniform_bind_signature]:
        uniform_bind_case = uniform.print_bind_case (mode)

        if uniform_bind_case not in uniform_bind_cases:
          uniform_bind_cases += [uniform_bind_case]

        if uniform_bind_case not in uniform_bind_case_shader_indices.keys ():
          uniform_bind_case_shader_indices[uniform_bind_case] \
              = [uniform.shader_index]
        else:
          uniform_bind_case_shader_indices[uniform_bind_case] \
              += [uniform.shader_index]

      for uniform_bind_case in uniform_bind_cases:
        for shader_index in uniform_bind_case_shader_indices[uniform_bind_case]:
          output_c_uniform_bind[mode] += "    case " + shader_index + ":\n"

        output_c_uniform_bind[mode] += uniform_bind_case + "      break;\n"

    if output_c_uniform_bind["integer"] != output_c_uniform_bind["float"]:
      output_c_uniform_bind_functions += \
          "void\n" + uniform_bind_signature + "\n{\n" \
          "  if (shader->is_integer) {\n" \
          "    switch (shader->index) {\n  " \
          + output_c_uniform_bind["integer"].replace ("\n", "\n  ") \
          + "    default:\n" \
          "        SCHRO_ASSERT (0);\n"\
          "        break;\n"\
          "    }\n" \
          "  } else {\n" \
          "    switch (shader->index) {\n  " \
          + output_c_uniform_bind["float"].replace ("\n", "\n  ") \
          + "    default:\n" \
          "        SCHRO_ASSERT (0);\n" \
          "        break;\n" \
          "    }\n" \
          "  }\n" \
          "}\n\n"
    else:
      output_c_uniform_bind_functions += \
          "void\n" + uniform_bind_signature + "\n{\n" \
          "  switch (shader->index) {\n" \
          + output_c_uniform_bind["integer"] \
          + "    default:\n" \
          "      SCHRO_ASSERT (0);\n"\
          "      break;\n"\
          "  }\n" \
          "}\n\n"

  output_c = output_c.replace ("===== UNIFORM BIND FUNCTIONS =====\n\n",
      output_c_uniform_bind_functions)

  file ("schroopenglshadercode.c", "wb").write (output_c)

