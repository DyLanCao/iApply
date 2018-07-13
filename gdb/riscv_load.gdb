define load_reg
	set {int}0x400b0000 = 1
	set {int}0x40102010 = 0x8000 
	load
	set {int}0x400b2000 = 0x8080
	set {int}0x400b0000 = 0
end
