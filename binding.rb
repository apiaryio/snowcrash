require 'ffi'

module Foo
  extend FFI::Library
  ffi_lib 'build/out/Release/obj.target/libsnowcrash'
  
  class MetadataCollection < FFI::Struct
    layout 	:meta_array, :pointer, 
			:size, :int
  end
  
  class Resources < FFI::Struct
    layout 	:resource_array, :pointer, 
			:size, :int
  end
  
  class Metadata < FFI::Struct
    layout 	:name, :string, 
			:value, :string
  end

  
  class Blueprint < FFI::Struct
    layout  :m_meta, MetadataCollection,
			:m_name, :string, 
			:m_desc, :string,
			:m_res, Resources
  end
  
  attach_function("parse", "C_parse", [ :string , :int ], :pointer)
  
end


pointer_blueprint = Foo.parse("meta: data\nfoo:bar\n# Api Name\n description for it" , 0 )

obj_blueprint = Foo::Blueprint.new(pointer_blueprint)

meta_1 = obj_blueprint[:m_meta]
res_1 = obj_blueprint[:m_res]

puts "name of the blueprint: " + obj_blueprint[:m_name]
puts "description of the blueprint: " + obj_blueprint[:m_desc]

puts "number of metadatas : " + meta_1[:size].to_s
puts "number of resources : " + res_1[:size].to_s

meta_obj1 = Foo::Metadata.new(meta_1[:meta_array] + (0 * Foo::Metadata.size))
meta_obj2 = Foo::Metadata.new(meta_1[:meta_array] + (1 * Foo::Metadata.size))

puts "metadata 1 key : " + meta_obj1[:name]
puts "metadata 1 value : " + meta_obj1[:value]

puts "metadata 2 key : " + meta_obj2[:name]
puts "metadata 2 value : " + meta_obj2[:value]
