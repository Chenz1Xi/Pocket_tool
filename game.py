import random

a = 0
b = 0
c = 0

def roll_dice():
    return random.randint(0, 1, 2)

for i in range(100):
    temp = 0
    temp = roll_dice()
    if temp == 0:
        a +=1
    elif temp == 1:
        b +=1
    else:
        c +=1

print("Dice rolls: ", a, b, c)