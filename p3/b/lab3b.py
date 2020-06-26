import sys
from collections import defaultdict


global exitcode
block_Map = defaultdict(list)
first_Node = 0
num_Node = 0
free_Node = []
alloc_Node = []
Link = dict()
Parent_I_Node = dict()


"""
0 ... successful execution, no inconsistencies found.
1 ... unsuccessful execution, bad parameters or system call failure.
2 ... successful execution, inconsistencies found.
"""


def get_Block_Data(file):
    file.seek(0)
    for line in file:
        data = line.split(',')

        if data[0] == 'SUPERBLOCK':
            Inode_Size = int(data[4])
            Block_Size = int(data[3])

        if data[0] == 'GROUP':
            count_Blocks = int(data[2])
            valid_Blocks = int(data[8]) + Inode_Size * int(data[3]) / Block_Size

        if data[0] == 'BFREE':
            block_Map[int(data[1])].append(['free'])

        if data[0] == 'INDIRECT':
            block_Map[int(data[5])].append(['', data[1], int(data[3])])

        if data[0] == 'INODE':
            for i in range(12, 27):
                if int(data[i]) == 0:
                    continue
                temp_str = ''
                if i == 24:
                    temp_str = 'INDIRECT'
                    offset = 12
                elif i == 25:
                    temp_str = 'DOUBLE INDIRECT'
                    offset = 268
                elif i == 26:
                    temp_str = 'TRIPLE INDIRECT'
                    offset = 65804
                else:
                    offset = i - 12
                block_Map[int(data[i])].append([temp_str, int(data[1]), offset])

    return valid_Blocks, count_Blocks

def easy_Write(ss, index0, block, index1, index2):
    if index0 != '':
        sp = ' BLOCK '
    else:
        sp = 'BLOCK '
    sys.stdout.write(ss + index0 + sp + str(block) + 
                ' IN INODE ' + str(index1) + ' AT OFFSET ' + str(index2) + '\n')

def process_Data(tt, cc):
    i = tt
    for blocknum in block_Map:
        if i <= blocknum:
            while i != blocknum and i < 64:
                sys.stdout.write("UNREFERENCED BLOCK " + str(i) + '\n')
                exitcode = 2
                i = i + 1
            i = i + 1
        else:
            continue

    array = []
    for blocknum, lst in block_Map.iteritems():
        temp_str = ""
        alloc_flag = 0
        free_flag = 0
        for item in lst:
            if item[0] == 'free':
                free_flag += 1
            else:
                alloc_flag += 1
                array.append(item)
            flag = 0
            if blocknum > 0 and blocknum < tt:
                flag = 1
                temp_str = 'RESERVED '
            elif blocknum < 0 or blocknum > cc:
                flag = 1
                temp_str = 'INVALID '
            if flag == 1:
                exitcode = 2
                easy_Write(temp_str, item[0], blocknum, item[1], item[2])

        if free_flag * alloc_flag >= 1:
            sys.stdout.write('ALLOCATED BLOCK '+str(blocknum)+' ON FREELIST'+'\n')
            exitcode = 2

        if alloc_flag > 1:
            for index in array:
                easy_Write('DUPLICATE ', index[0], blocknum, index[1], index[2])
                exitcode = 2

    



def INode_Data_Process(file):
    file.seek(0)
    for line in file:
        line = line[:-1]
        data = line.split(',') 

        if data[0] == 'SUPERBLOCK':
            first_Node = int(data[7])
            num_Node = int(data[2])
        if data[0] == 'IFREE':
            free_Node.append(int(data[1]))

        if data[0] == 'DIRENT':
            Num_I_Node = int(data[3])
            if Num_I_Node in Link:
                Link[Num_I_Node] += 1
            else:
                Link[Num_I_Node] = 1
            if Num_I_Node not in Parent_I_Node:
                Parent_I_Node[Num_I_Node] = int(data[1])

        if data[0] == 'INODE':
            Num_I_Node = int(data[1])
            alloc_Node.append(Num_I_Node)
            linkCnt = int(data[6])
            if Num_I_Node in free_Node:
                sys.stdout.write('ALLOCATED INODE ' + str(Num_I_Node) + ' ON FREELIST'+'\n')
                exitcode = 2
            
            NUM = "" 
            if Num_I_Node not in Link:
                NUM = "0"  
            elif Num_I_Node in Link and Link[Num_I_Node] != linkCnt: 
                NUM = str(Link[Num_I_Node])
            
            if NUM != "":
                sys.stdout.write('INODE ' + str(Num_I_Node) + 
                                ' HAS ' + NUM + 
                                ' LINKS BUT LINKCOUNT IS ' + str(linkCnt) + '\n')
                exitcode = 2

    file.seek(0)
    for line in file:
        line = line[:-1]
        data = line.split(',')

        if data[0] == 'DIRENT':
            Num_I_Node = int(data[3])
            dirNum   = int(data[1])
            
            temp_str = ""
            if Num_I_Node > num_Node or Num_I_Node < 1:
                temp_str = ' INVALID INODE '
            elif Num_I_Node not in alloc_Node and Num_I_Node in range (1,num_Node):
                temp_str = ' UNALLOCATED INODE '

            if temp_str != "":
                exitcode = 2
                sys.stdout.write('DIRECTORY INODE ' + str(dirNum) + 
                                 ' NAME ' + data[6] + temp_str + str(Num_I_Node) + '\n')
            temp_val = -1
            if data[6] == "'.'" and Num_I_Node != dirNum:
                temp_val = dirNum
            if data[6] == "'..'" and Num_I_Node != (Parent_I_Node[dirNum]):
                temp_val = str(Parent_I_Node[dirNum])
            if temp_val != -1:
                exitcode = 2
                sys.stdout.write('DIRECTORY INODE ' + str(dirNum) + ' NAME ' + data[6] + 
                                 ' LINK TO INODE ' + data[3] +' SHOULD BE ' + temp_val + '\n')

    for x in range(first_Node,num_Node):
        if x not in free_Node and x not in alloc_Node:
            sys.stdout.write('UNALLOCATED INODE ' + str(x) + ' NOT ON FREELIST' + '\n')
            exitcode = 2 



 
def main():
    exitcode = 0;
    if len(sys.argv) != 2:
        sys.stderr.write("must provide one argument: name of csv file" + '\n')
        exit(1)
    try:
        file = open(sys.argv[1])
    except IOError:
        sys.stderr.write("Error. Cannot open file" + '\n')
        exit(1)
    tt, CC = get_Block_Data(file)
    process_Data(tt, CC)
    INode_Data_Process(file)
    file.close()
    exit(exitcode)


if __name__ == '__main__':
    main()
