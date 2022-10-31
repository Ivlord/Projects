from os import path
from tkinter import *
from tkinter import colorchooser

edit_mode=False

class Field:
    class cell:
        def __init__(self, x, y, clr, C):
            self.x=x;       self.y=y
            self.clr='';    self.clr_next=clr
            self.C=C;       self.dot=0
            if clr!='':self.update()

        def update(self):
            if self.clr!=self.clr_next:
                self.clr=self.clr_next
                self.C.delete(self.dot)
                self.dot=self.C.create_oval(self.x*10, self.y*10, self.x*10+8, self.y*10+8, outline='', fill=self.clr)

    def __init__(self, root):
        root.title('Every Life Matters');
        root.resizable(False, False)                        #root.iconbitmap('icon.ico')  #root.overredirect(True)
        root.geometry('%dx%d+%d+%d' % (1000, 730, 250, 50));
        self.root  = root
        self.LRM_clr=['#FF0000','#00FF00','#000000']
        self.lab = [0]*4
        self.state = 1
        self.change_speed = 200
        self.mx=100;    self.my=70 # размер массива ячеек cell.

        self.but_state = Button(self.root, text = 'Pause', width = 10, command = self.state_change)
        self.but_state.place(x=20, y=705)

        self.score=Label(self.root, text = '', width = 5)
        self.score.place(x=110, y=705)

        if edit_mode:
            self.but_state_2 = Button(self.root, text = 'Save', width = 7, command = self.save_config)
            self.but_state_2.place(x=620, y=705)

            self.but_state_3 = Button(self.root, text = 'Load', width = 7, command = self.load_config)
            self.but_state_3.place(x=680, y=705)

        for i in range(1, 4):
            self.lab[i] = Label(self.root, text = 'But'+str(i), bg=self.LRM_clr[i-1], fg='grey', width = 5)
            self.lab[i].place(x=100+i*50, y=705)
            self.lab[i].bind('<Button-1>', lambda event, i=i: self.color_pick(event, i))

        self.lab[0]=Label(self.root, text = 'Screen', bg=self.LRM_clr[i-1], fg='grey', width = 8)
        self.lab[0].place(x=310, y=705)
        self.lab[0].bind('<Button-1>', lambda event: self.color_pick(event, 0))

        self.but_clear = Button(self.root, text = 'Clear', width = 10, command = self.clear_screen)
        self.but_clear.place(x=400, y=705)

        self.C=Canvas(self.root, width=1000, height=700, bg='#000001')
        self.C.place(x=0, y=0)
        [self.C.bind(b % n, lambda event, n=n: self.C_click(event, n))
        for b in ['<B%d-Motion>','<Button-%d>'] for n in [1,2,3]]
        self.root.bind("<MouseWheel>", self.mw)

        self.a=[[self.cell(x, y, '', self.C) for x in range(self.mx)] for y in range(self.my)]
        self.load_config()
        self.root.after(3000, self.run)

    def mw(self, event):
        if event.num == 5 or event.delta == -120:
            self.change_speed -= 100
        if event.num == 4 or event.delta == 120:
            self.change_speed += 100
        if self.change_speed < 100: self.change_speed = 100
        if self.change_speed > 2000: self.change_speed = 2000

    def byte3_to_tkinter(self, byte_3):
        d=0
        clr=hex(d.from_bytes(byte_3, byteorder='big'))[2:]
        clr='#'+'0'*(6-len(clr))+clr
        return '' if (clr=='#fffffe' or clr=='#FFFFFE') else clr

    def load_config(self):
        if not path.exists('elm.bin'): return
        file=open('elm.bin', 'rb')
        self.C.config(bg = self.byte3_to_tkinter(file.read(3)))
        for y in range(len(self.a)):
            for x in range(len(self.a[y])):
                r=file.read(3)
                if not r: return
                self.a[y][x].clr_next=self.byte3_to_tkinter(r)
                self.a[y][x].update()
        self.score['text']=str(self.colony_count())
        file.close()
        print('loaded')

    def save_config(self):
        file=open('elm.bin', 'wb+')
        file.write(int(self.C['bg'][1:], 16).to_bytes(3, byteorder='big'))
        for y in range(len(self.a)):
            for x in range(len(self.a[y])):
                tt=self.a[y][x].clr[1:]
                if tt:
                    bt = int(self.a[y][x].clr[1:], 16).to_bytes(3, byteorder='big')
                else:
                    bt = (0xfffffe).to_bytes(3, byteorder='big')
                file.write(bt)
        file.close()
        print('file saved')

    def clear_screen(self):
        for y in range(self.my):
            for x in range(self.mx):
                self.a[y][x].clr_next=''
                self.a[y][x].update()

    def color_pick(self, event, idx):
        if idx==3: return
        if idx==0:
            color_code=colorchooser.askcolor(title="Choose color for the screen")
            self.C.config(bg = color_code[1])
            self.lab[0].config(bg = color_code[1])
            return
        color_code=colorchooser.askcolor(title="Choose color for mouse button"+str(idx))
        if color_code[1]!=None:
            self.LRM_clr[idx-1]=color_code[1]
            self.lab[idx].config(bg = color_code[1])
            r=str(int(color_code[1][1:],16)^0xffffff)[:6]
            self.lab[idx].config(fg = '#'+ '0'*(6-len(r))+r )

    def C_click(self, event, idx):
        x, y=event.x//10, event.y//10
        if 0<=x<self.mx and 0<=y<self.my:
            if idx==3:
                self.a[y][x].clr_next=''
            else:
                self.a[y][x].clr_next=self.LRM_clr[idx-1]
            self.a[y][x].update()
            self.score['text']=str(self.colony_count())

    def cnt(self, xx, yy):
        return [int(self.a[y][x].clr[1:], 16) for y in range(yy-1, yy+2) for x in range(xx-1, xx+2)
        if ((xx,yy)!=(x,y)) and (0<=x<=self.mx-1) and (0<=y<=self.my-1) and self.a[y][x].clr!='']

    def colony_count(self):
        return sum([1 for y in range(self.my) for x in range(self.mx) if self.a[y][x].clr!=''])

    def run(self):
        if self.state==0: return
        for y in range(self.my):
            for x in range(self.mx):
                res=self.cnt(x, y)
                cnt=len(res)
                if self.a[y][x].clr!='' and (cnt not in [2,3]): # alive
                    self.a[y][x].clr_next=''
                elif self.a[y][x].clr=='' and cnt==3:
                    r=hex(sum(res)//(cnt))[2:]
                    self.a[y][x].clr_next='#'+ '0'*(6-len(r))+r

        [self.a[y][x].update() for y in range(self.my) for x in range(self.mx)]
        s=self.colony_count()
        self.score['text']=str(s)
        if not s:
            self.state_change()
            return
        self.root.after(self.change_speed, self.run)

    def state_change(self):
        self.state = not self.state
        self.but_state['text']=['Run', 'Pause'][self.state]
        if self.state: self.root.after(500, self.run)

def main():
    root = Tk();
    app = Field(root)
    root.mainloop()

if __name__ == '__main__':
    main()

#self.master.destroy()
#    def new_window(self):
#        self.newWindow = Toplevel(self.master)
#        self.app = Demo(self.newWindow)
