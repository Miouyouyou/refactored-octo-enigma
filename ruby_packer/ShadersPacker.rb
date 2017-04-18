class ShadersPacker

  GL_FRAGMENT_SHADER = 0x8B30
  GL_VERTEX_SHADER   = 0x8B31

  module ClassMethods
    def shader_name(name)
      "shaders/#{name}.vert\0shaders/#{name}.frag\0"
    end

    def shader_names(names)
      namelist = ""
      names.each {|name| namelist << shader_name(name) if name}
      namelist
    end

    def pack(names, out_filepath)
      count = names.length
      programs = Array.new(count, 0)
      namelist = shader_names(names)
      types = []
      names.each {|name|
        types << GL_VERTEX_SHADER
        types << namelist.index("shaders/#{name}.vert")
        types << GL_FRAGMENT_SHADER
        types << namelist.index("shaders/#{name}.frag")
      }
      content = programs.pack("I<*")
      content << types.pack("I<*")
      content << namelist.bytes.pack("C*") # Isn't there a better way ??
      File.write(out_filepath, content)
    end

  end

  extend ClassMethods

end

ShadersPacker.pack(
  %w{standard node fixed_widgets color_node}, "shaders.pack"
)
