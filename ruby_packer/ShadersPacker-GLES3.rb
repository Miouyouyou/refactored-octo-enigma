require 'fileutils'

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

    @@typenames = {GL_VERTEX_SHADER: "vert", GL_FRAGMENT_SHADER: "frag"}
    def index_of(namelist, name, type)
      namelist.index("shaders/#{name}.#{@@typenames[type]}")
    end

    def variable_location(declaration_line)
      location =
        declaration_line.scan(/\(\s*location\s*=\s*(\d+)\s*\)/).
          last.last
      if location then location.to_i else nil end
    end
    def uniforms_of(shaders_path)
      uniforms_names = []
      shaders_path.each {|filepath|
        File.open(filepath, "r") {|f|
          f.each_line {|shader_line|
            # This can get fooled by comments like : // uniform
            if (shader_line =~ /\buniform\b/)
              # The uniform name is the last run of letters in the
              # uniform line
              uniform_name = shader_line.scan(/\b\w+\b/).last
              # Manage locations places, when possible. Will be useful
              # in the near future, to generate enumerators
              # automatically

              # Scan return an array inside an array... Yo Dawg !
              location = variable_location(shader_line)

              if (location.nil?)
                uniforms_names << uniform_name
              else
                uniforms_names[location] = uniform_name
              end
            end
          }
        }
      }
      uniforms_names
    end

    def attributes_of(shaders_path)
      attributes_names = []
      File.open(shaders_path[0]) do |f|
        f.each_line do |shader_line|
          if (shader_line =~ /\b(in|attribute)\b/)
            attribute_name = shader_line.scan(/\b\w+\b/).last
            location = variable_location(shader_line)
            if (location.nil?)
              attributes_names << attribute_name
            else
              attributes_names[location] = attribute_name
            end
          end
        end
      end
      attributes_names
    end

    # Clear needs to be factorised
    def generate_enums(shaders_infos_hash)
      block_end = "}\n\n"
      glsl_shader_name_enum  = "enum glsl_shader_name {\n"
      glsl_program_name_enum = "enum glsl_program_name {\n"

      programs_elements_enums = []

      shaders_infos_hash.each do |shader_name, elements|

        glsl_shader_name_enum <<
          "\t#{shader_name}_vsh,\n\t#{shader_name}_fsh,\n"
        glsl_program_name_enum <<
          "\tglsl_program_#{shader_name},\n"

        program_attributes_enum = "enum #{shader_name}_attribute {\n"

        skip = false
        (0...elements[:attributes].length).each do |i|
          shader_attribute_name = elements[:attributes][i]
          if shader_attribute_name
            program_attributes_enum <<
              "\t#{shader_name}_shader_attr_#{shader_attribute_name}"
            program_attributes_enum << " = #{i}" if skip
            program_attributes_enum << ",\n"
            skip = false
          else
            skip = true
          end
        end

        program_attributes_enum << block_end

        program_uniforms_enum = "enum #{shader_name}_uniform {\n"

        skip = false
        (0...elements[:uniforms].length).each do |i|
          shader_uniform_name = elements[:uniforms][i]
          if shader_uniform_name
            program_uniforms_enum <<
              "\t#{shader_name}_shader_attr_#{shader_uniform_name}"
            program_uniforms_enum << " = #{i}" if skip
            program_uniforms_enum << ",\n"
            skip = false
          else
            skip = true
          end
        end

        program_uniforms_enum << block_end

        programs_elements_enums << program_attributes_enum
        programs_elements_enums << program_uniforms_enum
      end

      glsl_shader_name_enum  << block_end
      glsl_program_name_enum << block_end

      c_enums = ""
      c_enums << glsl_shader_name_enum
      c_enums << glsl_program_name_enum
      programs_elements_enums.each do |program_specific_enum|
        c_enums << program_specific_enum
      end
      
      c_enums
    end
    
    def shaders_enums(names, namelist)
      shaders_paths = namelist.split("\0").each_slice(2).to_a
      uniforms = {}
      names.each_with_index {|shader_name, i|
        uniforms[shader_name] = {
          attributes: attributes_of(shaders_paths[i]),
          uniforms: uniforms_of(shaders_paths[i]),
        }
      }
      generate_enums(uniforms)
    end

    def pack(names, out_filepath, shaders_root_path)
      FileUtils.cd(shaders_root_path)
      count = names.length
      programs = Array.new(count, 0)
      namelist = shader_names(names)
      shaders_enums = shaders_enums(names, namelist)
      puts shaders_enums
      types = []
      names.each {|name|
        types << GL_VERTEX_SHADER
        types << index_of(namelist, name, GL_VERTEX_SHADER)
        types << GL_FRAGMENT_SHADER
        types << index_of(namelist, name, GL_FRAGMENT_SHADER)
      }
      content = programs.pack("I<*")
      content << types.pack("I<*")
      content << namelist.bytes.pack("C*") # Isn't there a better way ??
      File.write(out_filepath, content)
    end

  end

  extend ClassMethods

end

p ARGV.length
if ARGV.length < 1
  abort("ShadersPackers.rb /path/to/shaders/directory")
end

ShadersPacker.pack(
  %w{lines node fixed_widgets color_node color_static},
  "data/shaders.pack",
  ARGV[0]
)
