import sys
class MemPoint:
    def __init__(self, time, heap, heap_extra, stack, heap_tree):
        self.time = int(time.split("=")[1])
        self.heap = int(heap.split("=")[1])
        self.heap_extra = int(heap_extra.split("=")[1])
        self.stack = int(stack.split("=")[1])

    def get_sum_memusage(self):
        return self.heap + self.heap_extra + self.stack

def get_mem_usage(filename):
    with open(filename) as file:
        contents = file.readlines()
    memory_points = []
    for index in range(len(contents)):
        if("snapshot" in contents[index]):
            emptyLine = contents[index+1] # not used
            time = contents[index+2] # not used
            mem_heap = contents[index+3]
            mem_heap_extra = contents[index+4]
            mem_stacks = contents[index+5]
            heap_tree = contents[index+6] #not used
            memory_points.append(MemPoint(time, mem_heap, mem_heap_extra, mem_stacks, heap_tree))
    maxUsage = max(value.get_sum_memusage() for value in memory_points)
    return maxUsage
    
if __name__ == "__main__":
    get_mem_usage(sys.argv[1])