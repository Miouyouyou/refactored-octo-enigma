a = StringIO.new(File.read("../data/shaders.pack"))
p a.read(5*4).unpack("I<*")
p a.read(13*4).unpack("I<*")
p a.read(10*8).unpack("I<*")
p a.read(5*8).unpack("S<*")
p a.read(228)
p a.read(118)

