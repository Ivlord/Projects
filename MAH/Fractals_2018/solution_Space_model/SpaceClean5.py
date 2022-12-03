import sys
import os
import pygame
from pygame.locals import *
import pygame.freetype
from math import sin as sin
from math import cos as cos
from math import radians as rad
import random
import time


def main():
    
    global screen, clock, GAME_FONT
    
    pygame.init()
    GAME_FONT = pygame.freetype.Font(resource_path('LucidaBrightRegular.ttf'), 18)
    image1 = pygame.image.load(resource_path('blackhole2.png'))

    BGColor=(0,0,0)
    
    screen_size =[1260, 940]
    pygame.display.set_caption("Фрактальная модель галактики")
    screen = pygame.display.set_mode(screen_size)
    screen.fill(BGColor)

    text_surface, rect = GAME_FONT.render('Иван Мазлов (с)', (255, 255, 0))
    #screen.blit(text_surface, (4, 4))	
    pygame.display.flip()
    clock = pygame.time.Clock()
    random.seed()

    GalaxyCenter=[int(screen_size[0]/2), int(screen_size[1]/2)]
    screen.blit(image1, (GalaxyCenter[0]-61, GalaxyCenter[1]-35)) 

    pattern = {
                'ОрбитаXмин'     : [120, 25, 15],
                'ОрбитаXмакс'    : [620, 90, 20],
                'ОрбитаYмин'     : [110, 25, 15],
                'ОрбитаYмакс'    : [470, 80, 20],
                'РазмерМин'      : [ 10,  5,  1],
                'РазмерМакс'     : [ 20,  7,  3],
                'СкоростьМин'    : [  1,  5, 12],
                'СкоростьМакс'   : [  3, 10, 25],
                'ТелНаОрбитеМин' : [  5,  0,  0],
                'ТелНаОрбитеМакс': [ 15,  8,  8]
               }
        

    Space=Create_New_Galaxy(pattern)

    running = True
    
    while running:
        clock.tick(10)
        
        Space=draw_planets(Space, GalaxyCenter, screen, (0,0,0))
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running=False
                break
            elif event.type == 5:
                Space=Create_New_Galaxy(pattern)
        #time.sleep(.2)
        Space=draw_planets(Space, GalaxyCenter, screen, -1)
        screen.blit(image1, (GalaxyCenter[0]-61, GalaxyCenter[1]-35))
        pygame.display.flip()
    pygame.quit()

def Create_New_Galaxy(pattern):
   Space=[ {'Цвет':(255,255,180), 'Угол':90,
            'РадиусX':0, 'РадиусY':0, 'Размер':1,  'Скорость':0,
            'Координаты':[100,100],
            'Спутники': setup_do(0, pattern)} ]
   return Space

def setup_do(level, pat):
    if level>2:
        return []
    
    suns =    [ (243, 247, 4), (247, 198, 4),(255, 253, 135), (249, 40, 14),
                (163, 175, 255), (247, 247, 247), (217, 235, 252), (188, 214, 255)
              ]

    sat_number=random.randint(pat['ТелНаОрбитеМин'][level], pat['ТелНаОрбитеМакс'][level])
    sats=[]
    
    for i in range(sat_number):
        sats.append({'Координаты':[100,100],
                     'Угол':     random.randint(0,360),
                     'РадиусX' : random.randint(pat['ОрбитаXмин'  ][level], pat[ 'ОрбитаXмакс'][level]),
                     'РадиусY' : random.randint(pat['ОрбитаYмин'  ][level], pat[ 'ОрбитаYмакс'][level]),
                     'Размер'  : random.randint(pat['РазмерМин'   ][level], pat['РазмерМакс'  ][level]),
                     'Скорость': random.randint(pat['СкоростьМин' ][level], pat['СкоростьМакс'][level])
                     })
        if level!=0:
            sats[i]['Цвет']= rnd_color()
        else:
            sats[i]['Цвет']=suns[random.randint(0, len(suns)-1)]
        sats[i]['Спутники']= setup_do(level+1, pat)
    return sats

def resource_path(relative_path):
    """ Get absolute path to resource, works for dev and for PyInstaller """
    base_path = getattr(sys, '_MEIPASS', os.path.dirname(os.path.abspath(__file__)))
    return os.path.join(base_path, relative_path)

def rnd_color():
    return (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))

def draw_planets(SpaceObjects_list, OrbitCenterXY_list, screen, color):
    result=[]
    for SAT in SpaceObjects_list:
        clr=color
        if color==-1:
            clr=SAT['Цвет']
            SAT['Угол']=SAT['Угол'] + SAT['Скорость']
            if SAT['Угол']>360:
                SAT['Угол']-=360
            SAT['Координаты']=[int(SAT['РадиусX'] * cos(rad(SAT['Угол'])) + OrbitCenterXY_list[0]),
                               int(SAT['РадиусY'] * sin(rad(SAT['Угол'])) + OrbitCenterXY_list[1])]
        pygame.draw.circle(screen, clr, SAT['Координаты'] , SAT['Размер'], 0)
        if len(SAT['Спутники'])>0:
            SAT['Спутники']=draw_planets(SAT['Спутники'], SAT['Координаты'],screen, color)
        result.append(SAT)
    return result

if __name__ == '__main__':
    main()

