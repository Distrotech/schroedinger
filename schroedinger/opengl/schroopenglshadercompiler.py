#!/usr/bin/python

import re
import sys

def error (message):
  print "ERROR: " + message
  exit (-1)

class Line:
  def __init__ (self, number, string):
    self.number = number
    self.level = 0
    self.content = ""
    self.version = "default"
    
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

    string += "(Line: " + repr (self.level) + ", " + repr (self.content) \
        + ", " + repr (self.version) + ")"

    return string

  def decrease_level (self):
    self.level = self.level - 1

class Block:
  def __init__ (self):
    self.header = None
    self.children = []
    self.version = "default"

  def __repr__ (self):
    string = ""
    level = self.header.level

    while level > 0:
      string += "  " 
      level = level - 1
      
    string += "(Block: " + repr (self.header).lstrip () + "\n"

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
  def __init__ (self):
    self.name = "unknown"
    self.version_keys = [] # to keep the keys in definition order
    self.versions = {}
    self.uniforms = []
    self.textures = []
    self.functions = []
    self.used_builtins = {} # keyed by version
    self.available_builtins = { "write_u8" : "SHADER_WRITE_U8", \
        "write_u8_vec4" : "SHADER_WRITE_U8_VEC4", \
        "write_s16" : "SHADER_WRITE_S16", \
        "divide_s16" : "SHADER_DIVIDE_S16", \
        "cast_s16_u8" : "SHADER_CAST_S16_U8", \
        "cast_u8_s16" : "SHADER_CAST_U8_S16", \
        "ref_weighting_s16" : "SHADER_REF_WEIGHTING_S16" }
    self.builtin_dependencies = { "ref_weighting_s16" : "divide_s16" }

  def __repr__ (self):
    string = "(Shader " + self.name + ":\n"

    # versions
    string += "  (Versions:\n"

    for key in self.version_keys:
      string += "    " + repr (key) + ": " + repr (self.versions[key]) \
          + ",\n"

    string = string.rstrip (",\n") + ")\n"

    # uniforms
    string += "  (Uniforms:\n"

    for uniform in self.uniforms:
      string += "    " + repr (uniform).strip () + ",\n"

    string = string.rstrip (",\n") + ")\n"

    # textures
    string += "  (Textures:\n"

    for texture in self.textures:
      string += "    " + repr (texture).strip () + ",\n"

    string = string.rstrip (",\n") + ")\n"

    # functions
    string += "  (Functions:\n"

    for function in self.functions:
      string += "    " + repr (function).lstrip ().replace ("\n", "\n  ") \
          + ",\n"

    string = string.rstrip (",\n") + ")"

    return string.rstrip ("\n") + ")"

  def __str__ (self):
    string = ""

    for version in self.version_keys:
      string += self.print_version (version)

    return string

  def parse (self, block):
    match = re.search ("^shader ([a-z0-9_]+):$", block.header.content)

    if match is None:
      error ("expecting header 'shader [a-z0-9_]+:' but found '" \
          + block.header.content + "' at line " + repr (block.header.number))

    self.name = match.group (1)
    children = block.children[:]

    # parse versions
    if not isinstance (children[0], Block):
      error ("expecting a block")
    
    if children[0].header.content != "versions:":
      error ("expecting 'versions:' as first block after " \
         "'shader [a-z0-9_]+:' but found '" + children[0].header.content \
         + "' at line " + repr (children[0].header.number))

    for child in children[0].children:
      if not isinstance (child, Line):
        error ("found non-Line in 'versions:' block")

      self.parse_version (child)

    children = self.transform_versions (children[1:])

    # parse uniforms/textures/functions
    while len (children) > 0:
      if not isinstance (children[0], Block):
        error ("expecting a block")

      if children[0].header.content == "uniforms:":
        for child in children[0].children:
          if isinstance (child, Block):
            child = self.transform_versions (child)

          if isinstance (child, Line):
            self.parse_uniform (child)
      elif children[0].header.content == "textures:":
        for child in children[0].children:
          if isinstance (child, Line):
            self.parse_texture (child)
      elif children[0].header.content.startswith("func "):
        self.parse_builtins (children[0])

        self.functions += [children[0]]
      else:
        error ("expection uniforms/textures/function block but found '" \
            + children[0].header.content + "'")

      children = children[1:]

    for version in self.used_builtins.keys ():
      additional_builtins = []

      for builtin in self.used_builtins[version]:
        if builtin in self.builtin_dependencies.keys ():
          additional_builtin = self.builtin_dependencies[builtin]
          
          if additional_builtin not in additional_builtins and \
            additional_builtin not in self.used_builtins[version]:
            additional_builtins += [additional_builtin]

      self.used_builtins[version] += additional_builtins

      self.used_builtins[version].sort ()

  def parse_version (self, line):
    match = re.search ("^([a-z0-9_]+)[ ]+([a-zA-Z0-9_]+)$", line.content)

    if match is None:
      error ("expecting version '[a-z0-9_]+[ ]+[a-zA-Z0-9_]+' but found '" \
          + line.content + "' at line " + repr (line.number))

    key = match.group (1)
    value = match.group (2)

    if key in ["default", "shader", "versions", "uniforms", "textures"] or \
        key.startswith ("func "):
      error ("version '" + key + "' is a reserved word [default, shader, " \
          "versions, uniforms, textures, func]")

    if key in self.version_keys:
      error ("version '" + key + "' already defined")

    self.version_keys += [key]
    self.versions[key] = value

  def parse_uniform (self, line):
    match = re.search ("^([a-z0-9]+) ([a-z0-9_]+)$", line.content)

    if match is None:
      error ("expecting uniform '[a-z0-9]+ [a-z0-9_]+' but found '" \
          + line.content + "' at line " + repr (line.number))

    self.uniforms += [line]

  def parse_texture (self, line):
    match = re.search ("^(u8|u8_vec4|s16)[ ]+([a-z0-9_]+)$", line.content)

    if match == None:
      error ("expecting texture '(u8|u8_vec4|s16)[ ]+[a-z0-9_]+' but found '" \
          + line.content + "' at line " + repr (line.number))

    self.textures += [line]

  def parse_builtins (self, block):
    for child in block.children:
      if isinstance (child, Line):
        for builtin in self.available_builtins.keys ():
          if re.search ("([^a-zA-Z0-9_]|^)" + builtin + "([^a-zA-Z0-9_]|$)", \
              child.content) is not None:
            if child.version == "default":
              versions = self.version_keys
            else:
              versions = [child.version]

            for version in versions:
              if version in self.used_builtins:
                if builtin not in self.used_builtins[version]:
                  self.used_builtins[version] += [builtin]
              else:
                self.used_builtins[version] = [builtin]
      elif isinstance (child, Block):
        self.parse_builtins (child)
      else:
        error ("internal type error")

  def transform_versions (self, children):
    transformed = []
    
    for child in children:
      if isinstance (child, Line):
        transformed += [child]
      elif isinstance (child, Block):
        key = child.header.content[:-1]

        if key in ["shader", "versions", "uniforms", "textures"] or \
            key.startswith ("func ") or key.startswith ("if ") or \
            key.startswith ("elif ") or key.startswith ("else"):
          child.children = self.transform_versions (child.children)
          transformed += [child]
        elif key not in self.version_keys:
          error ("version '" + key + "' is undefined")
        else:
          for subchild in child.children:
            if subchild.version != "default":
              error ("nesting versions is not allowed at line " \
                  + repr (child.number))

            subchild.decrease_level ()

            subchild.version = key

          transformed += self.transform_versions (child.children)
      else:
        error ("internal type error")

    return transformed

  def replace_vars (self, string, type):
    if type == "integer":
      string = re.sub ("([^a-zA-Z0-9_]?)var_u8 ", "\\1uint ", string)
      string = re.sub ("([^a-zA-Z0-9_]?)var_s16 ", "\\1int ", string)
      string = re.sub ("([^a-zA-Z0-9_]?)var4_u8 ", "\\1uvec4 ", string)
      string = re.sub ("([^a-zA-Z0-9_]?)var4_s16 ", "\\1ivec4 ", string)
    elif type == "float":
      string = re.sub ("([^a-zA-Z0-9_]?)var_u8 ", "\\1float ", string)
      string = re.sub ("([^a-zA-Z0-9_]?)var_s16 ", "\\1float ", string)
      string = re.sub ("([^a-zA-Z0-9_]?)var4_u8 ", "\\1vec4 ", string)
      string = re.sub ("([^a-zA-Z0-9_]?)var4_s16 ", "\\1vec4 ", string)
    else:
      error ("internal type error")

    return string

  def replace_numbers (self, string, type):
    if type == "float":
      string = re.sub ("((?:[+\\-*/=,( ]|^)[0-9]+)([^0-9\\.]|$)", "\\1.0\\2", \
          string)

    return string

  def print_version (self, version):
    string = "  { " + self.versions[version] + ",\n      \"" \
        + self.name + "/" + version \
        + "\", SHADER_FLAG_USE_U8 | SHADER_FLAG_USE_S16,\n" \

    # float
    string += "      SHADER_HEADER\n"

    # float - textures
    index = 1

    for texture in self.textures:
      if texture.version in ["default", version]:
        match = re.search ("^(u8|u8_vec4|s16)[ ]+([a-z0-9_]+)$", \
            texture.content)

        if match == None:
          error ("expecting texture '(u8|u8_vec4|s16)[ ]+[a-z0-9_]+' but " \
              + "found '" + texture.content + "' at line " \
              + repr (texture.number))

        if match.group (1) == "u8":
          string += "      SHADER_READ_U8 (\"texture" + str (index) \
              + "\", \"_" + match.group (2) + "\")\n"
        elif match.group (1) == "u8_vec4":
          string += "      SHADER_READ_U8_VEC4 (\"texture" + str (index) \
              + "\", \"_" + match.group (2) + "\")\n"
        elif match.group (1) == "s16":
          string += "      SHADER_READ_S16 (\"texture" + str (index) \
              + "\", \"_" + match.group (2) + "\")\n"
        else:
          error ("internal match error")

        index = index + 1

    # float - builtins
    if version in self.used_builtins.keys ():
      for builtin in self.used_builtins[version]:
        string += "      " + self.available_builtins[builtin] + "\n"

    # float - uniforms
    for uniform in self.uniforms:
      if uniform.version in ["default", version]:
        string += "      \"uniform " +  uniform.content + ";\\n\"\n"

    # float - functions
    for function in self.functions:
      string += self.print_function (function, version, "float")

    string = string.rstrip("\n") + ",\n"

    # integer
    string += "      SHADER_HEADER_INTEGER\n"

    # integer - textures
    index = 1

    for texture in self.textures:
      if texture.version in ["default", version]:
        match = re.search ("^(u8|u8_vec4|s16)[ ]+([a-z0-9_]+)$", \
            texture.content)

        if match == None:
          error ("expecting texture '(u8|u8_vec4|s16)[ ]+[a-z0-9_]+' but " \
              + "found '" + texture.content + "' at line " \
              + repr (texture.number))

        if match.group (1) == "u8":
          string += "      SHADER_READ_U8_INTEGER (\"texture" + str (index) \
              + "\", \"_" + match.group (2) + "\")\n"
        elif match.group (1) == "u8_vec4":
          string += "      SHADER_READ_U8_VEC4_INTEGER (\"texture" \
              + str (index) + "\", \"_" + match.group (2) + "\")\n"
        elif match.group (1) == "s16":
          string += "      SHADER_READ_S16_INTEGER (\"texture" + str (index) \
              + "\", \"_" + match.group (2) + "\")\n"
        else:
          error ("internal match error")

        index = index + 1

    # integer - builtins
    if version in self.used_builtins.keys ():
      for builtin in self.used_builtins[version]:
        string += "      " + self.available_builtins[builtin] + "_INTEGER\n"

    # integer - uniforms
    for uniform in self.uniforms:
      if uniform.version in ["default", version]:
        string += "      \"uniform " +  uniform.content + ";\\n\"\n"

    # integer - functions
    for function in self.functions:
      string += self.print_function (function, version, "integer")

    return string.rstrip(" \n") + " },\n"

  def print_function (self, function, version, type):
    match = re.search ("^func (.*):$", function.header.content)

    if match is None:
      error ("expecting function signature 'func .*:' but found '" \
          + function.header.content + "'")

    string = "      \"" + self.replace_vars (match.group (1), type) \
        + " {\\n\"\n"

    for child in function.children:
      if child.version not in ["default", version]:
        continue

      if isinstance (child, Line):
        string += self.print_function_line (child, version, type, "  ")
      elif isinstance (child, Block):
        result = self.print_function_block (child, version, type, "  ")        

        if result.startswith ("else "):
          string = string.rstrip ("\"\n\\n") + " " + result
        else:
          string += result
      else:
        error ("internal type error")

    return string + "      \"}\\n\"\n"

  def print_function_line (self, line, version, type, indent):
    string = "      \"" + indent + self.replace_vars (line.content, type) \
         + ";\\n\"\n"

    return self.replace_numbers (string, type)

  def print_function_block (self, block, version, type, indent):
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
      if child.version not in ["default", version]:
        continue

      if isinstance (child, Line):
        string += self.print_function_line (child, version, type, \
            indent + "  ")
      elif isinstance (child, Block):
        result = self.print_function_block (child, version, type, \
            indent + "  ")

        if result.startswith ("else "):
          string = string.rstrip ("\"\n\\n") + " " + result
        else:
          string += result
      else:
        error ("internal type error")

    return string + "      \"" + indent + "}\\n\"\n"

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

    shaders = []

    for block in blocks:
      shader = Shader ()

      shader.parse (block)

      shaders += [shader]

    preamble = file ("schroopenglshadercompiler.preamble", "rb").read ()
    string = "static struct ShaderCode schro_opengl_shader_code_list[] = {\n"

    for shader in shaders:
      string += str (shader)

    print "/* WARNING! Generated code, do not edit! */\n\n" + preamble \
        + string + "\n  { -1, NULL }\n};"

