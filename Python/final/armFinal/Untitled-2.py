
# initializing string 
test_string = [1,1]
  
# printing original string 
print("The original string : " +
      str(test_string)) 
  
# using int() 
# converting hexadecimal string to decimal 
#res = int(test_string, 16) 
res = int.from_bytes(test_string, byteorder='big', signed=False)
  
# print result 
print('The decimal number of hexadecimal \string\ : ' ,res)