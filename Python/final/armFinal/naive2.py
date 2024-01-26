def naive_conversion(x, base):
    y = ""
    if x==0:
        ab = ab.join(str(0),str(0))
        cd = cd.join(str(0),str(0))
    while x > 0:
        y = str(x % base) + y
        x //= base
        t = [int(d) for d in str(y)]

    return t

g=naive_conversion(0,16)
print(g)