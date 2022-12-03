import sys
import os
import pygame
from pygame.locals import *
import pygame.freetype
from math import sin as sin
from math import cos as cos
from math import radians as rad
from math import atan2 as atan
import random
import time


def resource_path(relative_path):
    """ Get absolute path to resource, works for dev and for PyInstaller """
    base_path = getattr(sys, '_MEIPASS', os.path.dirname(os.path.abspath(__file__)))
    print(base_path)
    return os.path.join(base_path, relative_path)

def main():
    pygame.init()
 
    global screen, clock, Cx, Cy, GAME_FONT
    BGColor=(0,0,0)
    screen_size =[1260, 940]  #[800, 500] #[1260, 940]
    screen = pygame.display.set_mode(screen_size)
    pygame.display.set_caption("Конструктор фракталов")
    font=resource_path('LucidaBrightRegular.ttf')
    GAME_FONT = pygame.freetype.Font(font, 28)
    #GAME_FONT = pygame.freetype.Font('LucidaBrightRegular.ttf', 28) #
    
    clock = pygame.time.Clock()
    Cx, Cy = int(screen_size[0]/2),int(screen_size[1]/2)
    CircleRadius=       130
    Growth=             0.5
    Surface=[  [[Cx,Cy],[Cx,Cy]] ]
    screen.fill(BGColor)
    pygame.display.flip()
    random.seed()
    running = True

    #0-Value 1-MaxLim 2-MinLim 3-Label 4-GeneralY  5-DataXoffset 6-Button1 7-Button2
    Settings=[
     [  3,   6,  3, 'Сторон  :',   0, 165,     ['<', 220,  -1, [] ], ['>', 248,  1, []] ],
     [  3,   6,  2, 'Итераций:',  32, 165,     ['<', 220,  -1, [] ], ['>', 248,  1, []] ],
     [180, 330,  0, 'Поворот :',  64, 165,     ['<', 220, -30, [] ], ['>', 248, 30, []] ],
     [100, 100, 30, 'Вероят. :',  96, 165,     ['<', 220, -10, [] ], ['>', 248, 10, []] ],
     [130, 300, 50, 'Размер  :', 128, 165,     ['<', 220, -10, [] ], ['>', 248, 10, []] ]
         ]
    

    Settings=menu(Settings)


    while running:
        
        clock.tick(100)
        for event in pygame.event.get():
            if event.type==5:
                result,Settings=menu_chck(Settings, pygame.mouse.get_pos())
                if result:
                    pygame.display.update(0,0,100,250)
                    Settings=menu(Settings)
                else:
                    screen.fill((BGColor))
                    #draw_fractal(screen, Surface, CircleRadius,  NumberOfParties, Growth, Iterations,    AppearProbability, TurnFactor, SleepTime): 
                    draw_fractal(screen, Surface, Settings[4][0], Settings[0][0], Growth, Settings[1][0], Settings[3][0], Settings[2][0], .5)
                    Settings=menu(Settings)
            if event.type == pygame.QUIT:
                running=False


        pygame.display.flip()
    pygame.quit()

def menu_chck(Settings, pos):
    for k in range(len(Settings)):
        for j in range(len(Settings[k])-2, len(Settings[k])):
            if (Settings[k][j][3][0]<pos[0]<Settings[k][j][3][2]) and (Settings[k][j][3][1]<pos[1]<Settings[k][j][3][3]):
                Settings[k][0]+=Settings[k][j][2]
                if Settings[k][0]>Settings[k][1] or Settings[k][0]<Settings[k][2]: 
                    Settings[k][0]-=Settings[k][j][2]
                return True, Settings
    return False, Settings

        
def rnd_color():
    return (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))


def rotate_dots(Y, Center, Dots):
    result = []
    for dot in Dots:
        dx, dy, = dot[0] - Center[0], dot[1] - Center[1]
        x= dx*cos(Y) - dy*sin(Y) + Center[0]
        y= dx*sin(Y) + dy*cos(Y) + Center[1]
        result.append( [int(x), int(y)] ) 
    return result


def draw_fractal(screen, Surface, CircleRadius, NumberOfParties, Growth, Iterations, AppearProbability, TurnFactor, SleepTime):
    for deep_rot in range(0,Iterations):
        NewLines, Color=[], rnd_color()
        
        for SurfaceLine in Surface:
            NewLines=NewLines + drawF(screen, SurfaceLine, CircleRadius, NumberOfParties, Color, 6-deep_rot, TurnFactor)
        
        clock.tick(100)
        pygame.display.flip()
        time.sleep(SleepTime)
        
        CircleRadius=int(CircleRadius * Growth)
        Surface=NewLines
        for u in range(len(Surface)-1, 0, -1):
            if random.randint(0, 100) > AppearProbability:
                del Surface[u]


def drawF(screen, Surface, CircleRadius, NumberOfParties, Color, dp, TurnFactor):
    
    SurfaceGamma=atan(Surface[1][1] - Surface[0][1], Surface[1][0] - Surface[0][0]) + rad(TurnFactor)
    if Surface[0]!=Surface[1]:
        FirstTop=[int((Surface[0][0]+Surface[1][0])/2), int((Surface[0][1]+Surface[1][1])/2) ]
        DescribedCircleCenter =[FirstTop[0], FirstTop[1] + CircleRadius]
    else:
        FirstTop=[Surface[0][0], Surface[0][1]+CircleRadius]
        DescribedCircleCenter=Surface[0]
    F=[rotate_dots(rad(360/NumberOfParties*i), DescribedCircleCenter, [FirstTop])[0] for i in range(0,NumberOfParties)]
    if Surface[0]!=Surface[1]:
        F=rotate_dots(SurfaceGamma, F[0], F)


    pygame.draw.lines(screen, Color, True, F, dp)
    
    New_Surfaces=[ [F[i],F[i+1]] for i in range(0,len(F)-1) ]
    New_Surfaces.append([F[-1],F[0]])
    return New_Surfaces


def menu(Settings):
    for k in range(len(Settings)):
        i=Settings[k]
        pygame.draw.rect(screen, (0, 255, 0), [0, i[4], 218, 30], 0)
        pygame.draw.rect(screen, (0, 0, 0),   [0, i[4], 218, 30], 2)
        text_surface, rect = GAME_FONT.render(i[3], (255, 0, 0))
        screen.blit(text_surface, (4, i[4]+4))

        text_surface, rect = GAME_FONT.render(str(i[0]), (255, 0, 0))
        screen.blit(text_surface, (i[5], i[4]+4))
        

        pygame.draw.rect(screen, (0, 200, 50), [i[6][1], i[4], 25, 30 ], 0)
        pygame.draw.rect(screen, (0, 0, 0),    [i[6][1], i[4], 25, 30 ], 2)
        text_surface, rect = GAME_FONT.render(i[6][0], (255, 0, 0))
        screen.blit(text_surface, (i[6][1]+5, i[4]+5))
        Settings[k][6][3]=[i[6][1], i[4], i[6][1]+25, i[4]+30 ]
        

        pygame.draw.rect(screen, (0, 200, 50), [i[7][1], i[4], 25, 30 ], 0)
        pygame.draw.rect(screen, (0, 0, 0),    [i[7][1], i[4], 25, 30 ], 2)
        text_surface, rect = GAME_FONT.render(i[7][0], (255, 0, 0))
        screen.blit(text_surface, (i[7][1]+5, i[4]+5))
        Settings[k][7][3]=[i[7][1], i[4], i[7][1]+25, i[4]+30 ]

        text_surface, rect = GAME_FONT.render('Иван Мазлов (с)', (255, 255, 0))
        screen.blit(text_surface, (4, 165))
    return Settings

if __name__ == '__main__':
    main()

