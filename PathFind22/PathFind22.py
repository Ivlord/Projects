import tkinter as tk

class pf(): #       tuples=> (x,y) (x,y)      Оne step:    Up    Right   Down   Left      Стенки и препятствия
    def __init__(self, maze, start, end, update_model = ((0,-1), (1,0), (0,1), (-1,0)), blocks = [1, '#', '@']):
        self.found, self.end, self.maze, self.update_model, self.paths = [], end, maze, update_model, [[start]]
        self.upd_len        = len(self.update_model)                   #      path0                path1
        self.dot_sum        = lambda A, B: (A[0] + B[0], A[1] + B[1]) # [ [(x0,y0),(x0,y0)], [(x,y),... ,(x,y)]]
        self.good = set([(x, y) for y in range(len(maze)) for x in range(len(maze[y])) if maze[y][x] not in blocks])

    def do(self): # Полностью прокручивает весь поиск пути
        while self.paths and not self.found: self.paths = self.one_step([])
        return self.found

    def one_step(self, new_paths = []): # Плюс один шаг на всех "живых" путях (добавляет к каждому пути ещё точки)
        for path in self.paths:
            new_dots = set(map(self.dot_sum, [path[-1]] * self.upd_len, self.update_model)) & self.good
            self.good -= new_dots
            if self.end in new_dots:
                self.found = path + [self.end]
                break
            new_paths += [path + [dot] for dot in new_dots]
        return new_paths

def maze_exits(maze, exit_marks=['ABCDEFGHIJKLMNOPQRSTUVWXYZ', 20]):
    return [(x,y) for y in range(len(maze)) for x in range(len(maze[y]))
                  if maze[y][x] in exit_marks[0] or maze[y][x] in exit_marks]

def draw_maze(maze, tkroot, btn_array, blocks = [1,'#','@'], exit_marks=['ABCDEFGHIJKLMNOPQRSTUVWXYZ', 20]):
    for y in range(len(maze)):
        for x in range(len(maze[y])):
            if maze[y][x] in blocks: fon='brown'
            elif maze[y][x] in exit_marks[0] or maze[y][x] in exit_marks:
                fon='red'
                btn_array[y][x].config(text=maze[y][x], fg='white')
            else: fon='grey'
            btn_array[y][x].config(bg=fon)
    root.update_idletasks()

def draw_pf(btn_array, pf=[], draw=True):
    if not pf: return
    for i in pf:
        if draw: btn_array[i[1]][i[0]].config(bg='orange')
        else: btn_array[i[1]][i[0]].config(bg='gray')
    root.update_idletasks()


maze = [#012345678901234567890123456
    "####B######################", #00
    "# #       #   #      #    #", #01
    "# # # ###### #### #######@#", #02 Добавлена стена @ : 23,1 - Выхода нет
    "# # # #       #   #       #", #03
    "#   # # ######### # ##### #", #04
    "# # # #   #       #     # #", #05
    "### ### ### ############# #", #06
    "# #   #     # #           #", #07
    "# # #   ####### ###########", #08
    "# # # #       # #         C", #09
    "# # ##### ### # # ####### #", #10
    "# # #     ### # #       # #", #11
    "#   ##### ### # ######### #", #12
    "######### ### #           #", #13 Только выход C: 22,13
    "# ####### ### #############", #14
    "A           #   ###   #   #", #15
    "# ############# ### # # # #", #16
    "# ###       # # ### # # # #", #17
    "# ######### # # ### # # # F", #18
    "#       ### # #     # # # #", #19
    "# ######### # ##### # # # #", #20
    "# #######   #       #   # #", #21
    "# ####### ######### #######", #22
    "#         #########       #", #23
    "#######E############D######"    #   len()=27x25
]


#################################################################################
start, cur_pf, a=[],[],[]

def but_one_press(event, x, y):
    clrs=['brown','grey','red']
    global start, cur_pf, maze
    cur_bg=event.widget.cget('bg')
    if RadioResult.get()==2:
        if cur_bg=='brown':
            if not start: status.config(text='Босс, надо выбрать стартовую позицию не в стене!')
            else:         status.config(text='Босс, это стена! Тут точно нет выхода!')
        elif cur_bg=='red':
            if not start: status.config(text='Вы сразу на выходе. Так нельзя! Ввозьмите клетку рядом!')
            else:         status.config(text='Увы, но в этот выход Вам не выйти!')
        elif cur_bg=='blue':
            a[start[1]][start[0]].config(bg='grey')
            start=[]
            draw_pf(a, pf=cur_pf, draw=False)
            for i in maze_exits(maze):
                a[i[1]][i[0]].config(bg='red')

        elif cur_bg=='green':
            if cur_pf: draw_pf(a, pf=cur_pf, draw=False)

            a[start[1]][start[0]].config(bg='blue')
            cur_pf=pf(maze, tuple(start), tuple([x,y])).do()
            cur_pf=cur_pf[1:]
            cur_pf=cur_pf[:-1]
            draw_pf(a, pf=cur_pf)

        else: #if grey or orange
            if start:
                a[start[1]][start[0]].config(bg='grey')
                draw_pf(a, pf=cur_pf, draw=False)
            start=[x, y]
            a[start[1]][start[0]].config(bg='blue')
            have_exits=[maze[ex[1]][ex[0]] for ex in maze_exits(maze) if pf(maze, tuple(start), ex).do()]
            have_exits_cord=[ex for ex in maze_exits(maze) if pf(maze, tuple(start), ex).do()]
            status.config(text='Доступны выходы: ' + ' '.join(sorted(have_exits))+'\nкликните на выход чтобы увидеть маршрут.' if have_exits else 'Выхода нет')
            for i in maze_exits(maze):
                a[i[1]][i[0]].config(bg='red')
            for i in have_exits_cord:
                a[i[1]][i[0]].config(bg='green')

            #root.update_idletasks()
    else: pass# TO_DO

def RadioChange():
    RB_sel=RadioResult.get()
    if RB_sel==1:
        status.config(text='Edit mode\nэта версия пока недоступна.')
        r2.select()
    else: status.config(text='Test mode')

root = tk.Tk()
root.geometry('1190x700')#"1300x500+0+0")
root.resizable(0, 0)
root.overrideredirect(1)

for y in range(len(maze)):
    a.append([])
    for x in range(len(maze[y])):
        a[y].append(tk.Button(root, text='', bg="orange", height=1, width=2, font=("Helvetica", 10, 'bold')))
        a[y][x].bind("<Button-1>", lambda event, x=x, y=y: but_one_press(event, x, y) )
        a[y][x].grid(row=y, column=x)

status=tk.Label(root,   text='',    height=5,   width=60,   justify=tk.LEFT, bg="yellow",
                        relief=tk.SOLID, borderwidth=1,     font=("Helvetica", 10, 'bold'))
status.grid(row=0, column=len(maze[0])+1, sticky = tk.NW, rowspan=4, columnspan=1) #, sticky = tk.W, rowspan=2, columnspan=20

RadioResult = tk.IntVar()
r1=tk.Radiobutton(root, text="Edit mode", padx=20, variable=RadioResult, value=1, command=RadioChange)
r2=tk.Radiobutton(root, text="Test mode", padx=20, variable=RadioResult, value=2, command=RadioChange)

r1.grid(row=4, column=len(maze[0]), rowspan=1, columnspan=3)
r2.grid(row=5, column=len(maze[0]), rowspan=1, columnspan=3)
r2.invoke()

draw_maze(maze, root, a, blocks = [1,'#','@'], exit_marks=['ABCDEFGHIJKLMNOPQRSTUVWXYZ', 20])

root.mainloop()
