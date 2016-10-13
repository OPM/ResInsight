#!/prog/sdpsoft/python2.6.7/bin/python


f= open('keywords.txt', "r")
for line in f:
    	tokens=line.split()
	fname = tokens[0]+".html"
	href = "<a href="+tokens[1]+">Click here to view help on the wiki page.</a>"
	file= open(fname, "w")
    	file.write (href)
	file.close()
f.close()