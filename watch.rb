#!/usr/bin/ruby

require "socket"
require "digest/sha1"

IP = Socket.ip_address_list.detect {|v| v.ipv4_private?}.ip_address
PORT = "8080"
CC = "emcc -c -std=c99"
LD = "emcc -o index.js"

class ServerHW

	def initialize
		@connection_list = []
		@server = TCPServer.new PORT
	end
	
	def listen
		begin
			conn = @server.accept_nonblock
			msg = ""
			while line = conn.gets and line != "\r\n"
				msg += line
			end

			if match = msg.match(/^GET (\S+)/)
				case match[1]
				when "/"
					file = File.read "index.html"
					split_file = file.split "<body>"
					ws_code = "
					<body>
					<script>
					var ws = new WebSocket(\"ws://#{IP}:#{PORT}/ws\")
					ws.onmessage = ev => window.location.reload(true)
					</script>
					"

					conn.write "HTTP/1.1 200\r\n"
					conn.write "Content-Type: text/html\r\n"
					conn.write "\r\n"
					conn.write split_file[0]
					conn.write ws_code
					conn.write split_file[1]
					conn.close
				when "/ws"
					if match = msg.match(/^Sec-WebSocket-Key: (\S+)/)
						key = Digest::SHA1.base64digest [match[1], "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"].join
		
						conn.write "HTTP/1.1 101 Switching Protocols\r\n"
						conn.write "Upgrade: websocket\r\n"
						conn.write "Connection: Upgrade\r\n"
						conn.write "Sec-WebSocket-Accept: #{key}\r\n"
						conn.write "\r\n"
						puts "#{Time.new.strftime("%Y-%m-%d %H:%M:%S")} connect: #{conn.peeraddr[3]}"
						@connection_list.append conn
					end
				else 
					file_name = match[1].split("/")[1] 
					begin
						file = File.binread file_name
						conn.write "HTTP/1.1 200\r\n"
						if file_name.include? ".js"
							conn.write "Content-Type: text/javascript\r\n"
						elsif file_name.include? ".wasm"
							conn.write "Content-Type: application/wasm\r\n"
						end
						conn.write "\r\n"
						conn.write file
					rescue Errno::ENOENT
						conn.write "HTTP/1.1 404\r\n"
						conn.write "\r\n"
					end
					conn.close
				end
			end
		rescue IO::EAGAINWaitReadable
		end
	end

	def reconnect
		for con in @connection_list 
			begin
				con.write [0b10000010, 1, 0].pack "ccc*"
			rescue Errno::ENOTCONN, Errno::ECONNRESET
			end
		end
		@connection_list.clear
	end

end

class SourceFile

	attr_reader :file_name, :time_stamp, :include_file
	
	def initialize file
		@include_file = []
		@file_name = file
		Dir.chdir "src/"
		@time_stamp = File.mtime file
		Dir.chdir "../"
	end

	def == other
		if self.file_name == other.file_name
			if self.time_stamp == other.time_stamp
				true
			else
				false	
			end
		else
			false
		end
	end

	def to_s
		"name: #@file_name\ninclude: #@include_file"
	end

	def set_include source
		Dir.chdir "src/"
		file = File.open source, 'r'
		Dir.chdir "../"
		read = file.read 512
		file.close
		if read.include? @file_name
			@include_file.append SourceFile.new source
		end
	end
end

def generate_list
	file_list = []
	Dir.chdir "src/"
	source = {}
	source["c"] = Dir.glob "*.c"
	source["h"] = Dir.glob "*.h"
	Dir.chdir '../'
	for file in source["h"]
		f = SourceFile.new file
		for src in source["c"]
			f.set_include src
		end
		file_list.append f
	end
	for file in source["c"]
		file_list.append SourceFile.new file
	end

	file_list
end

def clean_obj
	Dir.chdir "src/"
	c_files = Dir.glob("*.c").join " "
	Dir.chdir "../"
	Dir.chdir "obj/" 
	o_files = Dir.glob "*.o"

	for o in o_files
		if !c_files.include? o[0...-1] 
			File.delete o if File.exist? o
		end
	end
	Dir.chdir "../"
end

def compile file_list
	for file in file_list
		file_name = file.file_name
		if file_name.include? '.c'
			puts "#{Time.new.strftime("%Y-%m-%d %H:%M:%S")} compile: #{file_name}"
			system "#{CC} src/#{file_name} -o obj/#{file_name[0...-1]}o"
		end
	end
	clean_obj
	system "#{LD} #{Dir.glob("obj/*.o").join " "}"
end

puts "server run on http://#{IP}:#{PORT}"

files = generate_list
compile files

srv = ServerHW.new
run = true
while run
	for file in files
		f = SourceFile.new file.file_name
		if file != f
			if file.include_file.length == 0
				compile [file]
			else
				compile file.include_file
			end
			files = generate_list
			srv.reconnect
		end
	end
	srv.listen
	sleep 0.01
end
