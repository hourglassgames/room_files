import puzzletemplate as pz
import RPi.GPIO as GPIO
from time import sleep
from subprocess import run
from multiprocessing import Process
import PySimpleGUI as sg

def blankwindow():
    background_layout = [[]]
    background_window = sg.Window("background", background_layout, size=(800,480), finalize=True, background_color="black")
    background_window.maximize()
    while True:
        event, values = background_window.read()
        if event == sg.WIN_CLOSED or event == 'Exit':
            break
GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)
GPIO.setup(10, GPIO.IN,pull_up_down=GPIO.PUD_DOWN)

# This script calls for puzzle sequences to be created from the puzzletemplate.py script.
# Simply call the newwindow() function with any of the following parameters:
# windowname, default is 'window'
# backgroundimg, default is 'brickbackground.png'
# buttoncolor, default is '#000000'
# code, default is [9,9,9,9,9]
# buttonpadding, default is (10,130)
# buttonfont, default is ("Arial", 175)
# buttonsize, default is (15,13)
# windowsize, default is (800,480)
# digits, default is 3

# pz.newwindow()
# pz.newwindow('test', code=[2,4,5,7,9], digits=5)
# pz.newwindow('2nd Puzzle',code=[1,3,5,7], digits=4, backgroundimg='citybackground.png', buttoncolor='#c6d8ee')


def mainwindow():
    relaybuffer = 0
    run('vcgencmd display_power 0', shell=True)
    sleep(3)
    background_window = sg.Window("background", [[]], size=(800,505), background_color="black",location=(0,-25), finalize=True)
    background_window.maximize()

    while True:
        if GPIO.input(10) == GPIO.HIGH:
            relaybuffer = relaybuffer + 1
            print('signal received')
            if relaybuffer == 5:
                print('Button pushed')
                break
    run('vcgencmd display_power 1', shell=True)
    #pz.newwindow('calendar',titletxt='The Year Jack Was Born',titletxtfont=("Arial",50),cluetxt='', digits=4, code=[1,9,5,9],)
    #pz.newwindow('sudoku', titletxt='I LOVE Sudoku!',cluetxt='',digits=5,code=[6,5,5,2,7])
    #pz.newwindow('books',titletxt='Good Reads',cluetxt='My favorite Authors make Words on the Page come to life!',cluetxt2='(Reuland,49,193),(Bauer,92,69),(Coulter,60,254),(Walsh,7,41)',cluetxtfont=('Arial',15),digits=4, code=[1,3,4,5])
    #pz.newwindow('wordsearch',titletxt='Numbers in Letters',cluetxt='A decoder makes the search easier!',cluetxtfont=('Arial',16),titletxtfont=('Arial',50),code=[2,4,7,9,0],digits=5)

# pz.newwindow('blueprint',backgroundimg='blueprint.png',digits=4,code=[5,9,5,7])
# pz.newwindow('undercover',backgroundimg='undercover.png', digits=4, code=[3,7,5,0])
# pz.newwindow('plants', backgroundimg='plants.png',code=[7,2,6,5], digits=4)
#    pz.newwindow('cryptex',digits=0, bgcolor='light gray',txtcolor='black',titletxt='Cryptex',titletxtfont=('Arial',80),cluetxt='These are a few of my favorite RINGS',txt0='?',txt0color='black',txt1='2',txt1color='blue',txt2='?',txt2color='dark gray',txt3='1',txt3color='white',txt4='?',txt4color='orange')

#p1 = Process(target=blankwindow)
p2 = Process(target=mainwindow)

if __name__ == '__main__':
    #p1.start()
    sleep(.1)
    p2.start()
