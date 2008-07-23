#!/usr/bin/python

import re
import sys



def error (message):
  print "ERROR: " + message
  exit (-1)



def intersection (a, b):
  c = []

  for item in a:
    if item in b:
      c += [item]

  return c



class Item:
  def __init__ (self):
    self.versions = ["default"]

  def add_versions (self, versions):
    if "default" in self.versions:
      self.versions.remove ("default")

    self.versions += versions

  def has_version (self, versions):
    #for version in self.versions:
    #  if version in versions:
    #    return True

    #return False
    return len (intersection (versions, self.versions)) > 0



class Line (Item):
  def __init__ (self, number, string):
    Item.__init__(self)

    self.number = number
    self.level = 0
    self.content = ""
    
    match = re.search ("^([ ]*)([^ ].*)$", string)

    if match is not None:
      if len (match.group (1)) % 2 != 0:
        error ("invalid indentation in line " + repr (number) + " '" + string \
            + "'")

      self.level = len (match.group (1)) / 2
      self.content = match.group (2)

  def __repr__ (self):
    string = ""
    level = self.level

    while level > 0:
      string += "  " 
      level = level - 1

    string += "(Line: " + repr (self.number) + ", " + repr (self.level) + ", " \
        + repr (self.content) + ", " + repr (self.versions) + ")"

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
      
    string += "(Block: " + repr (self.versions) + " " \
        + repr (self.header).lstrip () + "\n"

    for child in self.children:
      string += repr (child) + "\n"

    return string.rstrip ("\n") + ")"

  def parse (self, lines):
    if len (lines) < 2 or lines[0].level + 1 != lines[1].level:
      return

    self.header = lines[0]
    lines = lines[1:]

    if not self.header.content.endswith (":"):
      error ("expecting ':' at end of line " + repr (self.header.number) \
          + " but found '" + self.header.content + "'")

    while len (lines) > 0 and self.header.level + 1 == lines[0].level:
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



class Shader:
  def __init__ (self, bunch, version, define):
    self.bunch = bunch
    self.version = version
    self.define = define
    self.uniforms = []
    self.textures = {}
    self.texture_order = []
    self.functions = []
    self.used_builtins = []
    self.available_builtins = { \
        "write_u8"            : "SHADER_WRITE_U8", \
        "write_u8_raw"        : "SHADER_WRITE_U8_RAW", \
        "write_vec4_u8"       : "SHADER_WRITE_VEC4_U8", \
        "write_vec4_u8_raw"   : "SHADER_WRITE_VEC4_U8_RAW", \
        "write_s16"           : "SHADER_WRITE_S16", \
        "write_s16_raw"       : "SHADER_WRITE_S16_RAW", \
        "write_vec4_s16"      : "SHADER_WRITE_VEC4_S16", \
        "write_vec4_s16_raw"  : "SHADER_WRITE_VEC4_S16_RAW", \
        "cast_s16_u8"         : "SHADER_CAST_S16_U8", \
        "cast_u8_s16"         : "SHADER_CAST_U8_S16", \
        "convert_raw_u8"      : "SHADER_CONVERT_RAW_U8", \
        "convert_raw_s16"     : "SHADER_CONVERT_RAW_S16", \
        "divide_s16"          : "SHADER_DIVIDE_S16", \
        "crossfoot_s16"       : "SHADER_CROSSFOOT_S16", \
        "ref_weighting_s16"   : "SHADER_REF_WEIGHTING_S16", \
        "biref_weighting_s16" : "SHADER_BIREF_WEIGHTING_S16" }
    self.builtin_dependencies = { \
        "ref_weighting_s16"   : "divide_s16", \
        "biref_weighting_s16" : "divide_s16" }
    self.read_calls = {} # keyed by texture key
    self.type_mappings = { \
        "integer" : { \
          "var_u8"   : "uint", \
          "var_s16"  : "int", \
          "var4_u8"  : "uvec4", \
          "var4_s16" : "ivec4" }, \
        "float"   : { \
          "var_u8"   : "float", \
          "var_s16"  : "float", \
          "var4_u8"  : "vec4", \
          "var4_s16" : "vec4" } }

  def __str__ (self):
    string = "  { " + self.define + ",\n      \"" + self.bunch + "/" \
        + self.version + "\",\n" \

    string += self.print_type ("float", "")
    string = string.rstrip("\n") + ",\n"
    string += self.print_type ("integer", "_INTEGER")

    return string.rstrip(" \n") + " },\n"

  def parse (self, children):
    # parse uniforms/textures/functions
    while len (children) > 0:
      if not isinstance (children[0], Block):
        error ("expecting a block")

      if children[0].header.content == "uniforms:":
        for child in children[0].children:
          if not isinstance (child, Line):
            error ("expecting lines in the uniforms block")

          self.parse_uniform (child)
      elif children[0].header.content == "textures:":
        for child in children[0].children:
          if not isinstance (child, Line):
            error ("expecting lines in the textures block")

          self.parse_texture (child)
      elif children[0].header.content.startswith("func "):
        self.parse_builtins (children[0])
        self.parse_read_calls (children[0])

        self.functions += [children[0]]
      else:
        error ("expection uniforms/textures/function block but found '" \
            + children[0].header.content + "'")

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

  def parse_uniform (self, line):
    if line.has_version (["default", self.version]):
      match = re.search ("^[a-z0-9]+[ ]+[a-z0-9_]+$", line.content)

      if match is None:
        error ("expecting uniform '[a-z0-9]+[ ]+[a-z0-9_]+' but found '" \
            + line.content + "' at line " + repr (line.number))

      self.uniforms += [line]

  def parse_texture (self, line):
    if line.has_version (["default", self.version]):
      match = re.search ("^(u8|s16)[ ]+([a-z0-9_]+)$", line.content)

      if match == None:
        error ("expecting texture '(u8|s16)[ ]+[a-z0-9_]+' but found '" \
            + line.content + "' at line " + repr (line.number))

      key = match.group (1) + " " + match.group (2)

      self.textures[key] = line
      self.texture_order += [key]

  def parse_builtins (self, block):
    for child in block.children:
      if isinstance (child, Line):
        if child.has_version (["default", self.version]):
          for builtin in self.available_builtins.keys ():
            if re.search ("([^a-zA-Z0-9_]|^)" + builtin + "([^a-zA-Z0-9_]|$)", \
                child.content) is not None and \
                builtin not in self.used_builtins:
              self.used_builtins += [builtin]
      elif isinstance (child, Block):
        if child.has_version (["default", self.version]):
          self.parse_builtins (child)
      else:
        error ("internal type error")

  def parse_read_calls (self, block):
    for child in block.children:
      if isinstance (child, Line):
        if child.has_version (["default", self.version]):
          string = child.content
          read_call_re = re.compile ("(?:[^a-zA-Z0-9_]|^)" \
              "read_([a-z0-9_]+?)(_vec4|)_(u8|s16)(_raw|)(?:[^a-zA-Z0-9_]|$)")
          match = read_call_re.search (string)

          while match is not None:
            define = "SHADER_READ"

            if match.group (2) == "_vec4":
              define += "_VEC4"

            if match.group (3) == "u8":
              define += "_U8"
            elif match.group (3) == "s16":
              define += "_S16"
            else:
              error ("internal match error")

            if match.group (4) == "_raw":
              define += "_RAW"

            key = match.group (3) + " " + match.group (1)

            if key not in self.textures.keys ():
              error ("can't read from undefined texture '" + key \
                  + "' at line " + repr (child.number))

            if key not in self.read_calls.keys ():
              self.read_calls[key] = (define, match.group (3), \
                  match.group (1), child)

            string = string[match.span ()[1]:]
            match = read_call_re.search (string)

          string = child.content
          copy_call_re = re.compile ("(?:[^a-zA-Z0-9_]|^)" \
              "copy_([a-z0-9_]+)_(u8|s16)(?:[^a-zA-Z0-9_]|$)")
          match = copy_call_re.search (string)

          while match is not None:
            define = "SHADER_COPY"

            if match.group (2) == "u8":
              define += "_U8"
            elif match.group (2) == "s16":
              define += "_S16"
            else:
              error ("internal match error")

            key = match.group (2) + " " + match.group (1)

            if key not in self.textures.keys ():
              error ("can't copy from undefined texture '" + key \
                  + "' at line " + repr (child.number))

            if key not in self.read_calls.keys ():
              self.read_calls[key] = (define, match.group (2), \
                  match.group (1), child)

            string = string[match.span ()[1]:]
            match = copy_call_re.search (string)
      elif isinstance (child, Block):
        if child.has_version (["default", self.version]):
          self.parse_read_calls (child)
      else:
        error ("internal type error")

  def replace_vars (self, string, type):
    if type in self.type_mappings.keys ():
      for key in self.type_mappings[type].keys ():
        string = re.sub ("([^a-zA-Z0-9_]|^)" + key + " ", \
            "\\1" + self.type_mappings[type][key] + " ", string)
    else:
      error ("internal type error")

    return string

  def replace_numbers (self, string, type):
    if type == "float":
      string = re.sub ("((?:[+\\-*/=,( ]|^)[0-9]+)([^0-9u\\.]|$)", "\\1.0\\2", \
          string)

    return string

  def print_type (self, type, postfix):
    string = "      SHADER_HEADER" + postfix + "\n"

    # textures/readcalls
    index = 1

    for key in self.texture_order:
      read_call = self.read_calls[key]
      string += "      " + read_call[0] + postfix + " (\"texture" \
          + str (index) + "\", \"_" + read_call[2] + "\")\n"
      index = index + 1

    # builtins
    for builtin in self.used_builtins:
      string += "      " + self.available_builtins[builtin] + postfix + "\n"

    # uniforms
    for uniform in self.uniforms:
      string += "      \"uniform " +  uniform.content + ";\\n\"\n"

    # functions
    for function in self.functions:
      string += self.print_function (function, type)

    return string

  def print_function (self, function, type):
    match = re.search ("^func (.*):$", function.header.content)

    if match is None:
      error ("expecting function signature 'func .*:' but found '" \
          + function.header.content + "'")

    string = "      \"" + self.replace_vars (match.group (1), type) \
        + " {\\n\"\n"

    for child in function.children:
      if isinstance (child, Line):
        if child.has_version (["default", self.version]):
          string += self.print_function_line (child, type, "  ")
      elif isinstance (child, Block):
        if child.has_version (["default", self.version]):
          result = self.print_function_block (child, type, "  ")

          if result.startswith ("else "):
            string = string.rstrip ("\"\n\\n") + " " + result
          else:
            string += result
      else:
        error ("internal type error")

    return string + "      \"}\\n\"\n"

  def print_function_line (self, line, type, indent):
    string = "      \"" + indent + self.replace_vars (line.content, type) \
         + ";\\n\"\n"

    return self.replace_numbers (string, type)

  def print_function_block (self, block, type, indent):
    match = re.search ("^(if|elif|else)[ ]*(.*):$", block.header.content)

    if match is None:
      error ("expecting function signature '(if|elif|else) .*:' but found '" \
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
        if child.has_version (["default", self.version]):
          string += self.print_function_line (child, type, indent + "  ")
      elif isinstance (child, Block):
        if child.has_version (["default", self.version]):
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
    self.defines = {} # keyed by version
    self.shaders = []

  def __str__ (self):
    string = ""

    for shader in self.shaders:
      string += str (shader)

    return string

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
      shader = Shader (bunch, version, self.defines[version])

      shader.parse (children[:])

      self.shaders += [shader]

  def parse_version (self, line):
    match = re.search ("^([a-z0-9_]+)[ ]+([a-zA-Z0-9_]+)$", line.content)

    if match is None:
      error ("expecting version '[a-z0-9_]+[ ]+[a-zA-Z0-9_]+' but found '" \
          + line.content + "' at line " + repr (line.number))

    version = match.group (1)
    define = match.group (2)

    if self.contains_reserved_word ([version]):
      error ("version '" + version + "' is a reserved word")

    if version in self.versions:
      error ("version '" + version + "' already defined")

    self.versions += [version]
    self.defines[version] = define

  def contains_reserved_word (self, list):
    for item in list:
     if item in ["shader", "versions", "uniforms", "textures", "else"] or \
         item.startswith ("func ") or item.startswith ("if ") or \
         item.startswith ("elif "):
       return True

    return False

  def transform_versions (self, children):
    transformed = []

    for child in children:
      if isinstance (child, Line):
        transformed += [child]
      elif isinstance (child, Block):
        if self.contains_reserved_word ([child.header.content[:-1]]):
          child.children = self.transform_versions (child.children)
          transformed += [child]
        else:
          versions = []
          version_re = re.compile ("^([a-z0-9_]+)[ ]*[,:][ ]*")
          string = child.header.content
          match = version_re.search (string)

          while match is not None:
            versions += [match.group (1)]
            string = string[match.span ()[1]:]
            match = version_re.search (string)

          if self.contains_reserved_word (versions):
            if len (versions) > 1:
              error ("versions " + repr (versions) + " contain a reserved word")
            else:
              child.children = self.transform_versions (child.children)
              transformed += [child]
          elif intersection (versions, self.versions) != versions:
            error ("at least one versions " + repr (versions) + " is undefined")
          else:
            for subchild in child.children:
              if subchild.versions != ["default"]:
                error ("nesting versions is not allowed at line " \
                    + repr (child.number))

              subchild.decrease_level ()
              subchild.add_versions (versions)

            transformed += self.transform_versions (child.children)
      else:
        error ("internal type error")

    return transformed



if __name__ == "__main__":
  for argument in sys.argv[1:]:
    lines = []
    number = 1

    for string in file (argument, "rb").readlines ():
      if '#' in string:
        string = string[0:string.index ('#')]

      string = string.rstrip ().replace("\t", " ")
      string = re.sub ("[ ]+:", ":", string)

      if len (string) > 0:
        lines += [Line (number, string)]

      number = number + 1

    blocks = []

    while len (lines) > 0:
      block = Block ()
      lines = block.parse (lines)

      blocks += [block]

    shader_bunches = []

    for block in blocks:
      shader_bunch = ShaderBunch ()

      shader_bunch.parse (block)

      shader_bunches += [shader_bunch]

    preamble = file ("schroopenglshadercompiler.preamble", "rb").read ()
    string = "static struct ShaderCode schro_opengl_shader_code_list[] = {\n"

    for shader_bunch in shader_bunches:
      string += str (shader_bunch)

    print "/* WARNING! Generated code, do not edit! */\n\n" + preamble \
        + string + "\n  { -1, NULL }\n};"

