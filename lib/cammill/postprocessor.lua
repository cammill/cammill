require("math")

about_text = ""

os.setlocale("C")

ModalTable = {}

function trim(s)
	return s:find'^%s*$' and '' or s:match'^%s*(.*%S)'
end
 
post = {}
post.Text = function (...)
  local arg={...}
	for i,v in ipairs(arg) do
		out = string.format("%s", v)
		append_output(out)
	end
end

post.ModalText = function (str)
---	if (ModalTable.txt == str) then
---	else
		append_output(str)
---	end
---	ModalTable.txt = str
end

post.Number = function (val, nformat)
	if (nformat == "0") then
		out = string.format("%i", val)
	elseif (nformat == nil) then
		out = string.format("%f", val)
	else
		out = ""
		vstr = string.format("%0.10f", val)
		fstr = nformat:sub(3,-1)
		bs = ""
		bds = ""
		ads = ""
		ad = 1
		d = 0
		fr = 0
		for c in vstr:gmatch"." do
			if (c == ".") then
				d = 1
			else
				if (d == 1) then
					if (fstr:sub(ad,ad) == "#") then
						if (c == "0") then
							bs = bs..c
						else
							ads = ads..bs..c
							bs = ""
						end
					elseif (fstr:sub(ad,ad) == "0") then
						ads = ads..c
					end
					if (frc == "#") then
						fr = ad
					end
					ad = ad + 1
				else
					bds = bds..c
				end
			end
		end
		out = bds
		if (ads == "") then
		else
			out = out.."."..ads
		end
	end
	append_output(out)
end

post.ModalNumber = function (str, val, nformat)
	mstr = trim(str)
	if (ModalTable[mstr] == val) then
	else
		out = string.format("%s", str)
		append_output(out)
		post.Number(val, nformat)
	end
	ModalTable[mstr] = val
end

post.NonModalNumber = function (str, val, nformat)
	out = string.format("%s", str)
	append_output(out)
	post.Number(val, nformat)
end

post.Eol = function ()
	append_output("\n")
end

post.Warning = function (str)
	io.write("(WARNING: ", str, ")\n")
end

post.SetCommentChars = function (str)
--	io.write("(SetCommentChars: ", str, ")\n")
end

post.ForceExtension = function (str)
	set_extension(str)
end

post.ArcAsMoves = function (str)
end

event = {}
function event:GetTextCtrl ()
	local res = o or {}
	setmetatable(res, self)
	self.__index = self
	about_text = ""
	ModalTable = nil
	ModalTable = {}
	return res
end

function event:AppendText (str)
	about_text = about_text..str
	output_info(about_text)
end

function load_post ()
	dofile(postfile)
end
function reset_modal ()
	ModalTable = nil
	ModalTable = {}
end







