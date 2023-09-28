x = input("Enter number > 256")
big = input("Big? True or false")
h = str(hex(x))
s = h.lstrip("0x")
e = s.zfill(4)
if big:
    f = e[0]
    g = e[1]
    h = e[2]
    i = e[3]
else:
    f = e[2]
    g = e[3]
    h = e[0]
    i = e[1]
j = "0x"
a = j+f+g
b = j+h+i
q = [a,b]
print (q[0])
print (q[1])

