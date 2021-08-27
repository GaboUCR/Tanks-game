import pygame
import socket
import selectors
import time
from pygame.locals import (
    K_UP,
    K_DOWN,
    K_LEFT,
    K_RIGHT,
    K_ESCAPE,
    KEYDOWN,
    QUIT,
)
from enum import Enum

# class msg(Enum):


class Player(pygame.sprite.Sprite):
    def __init__(self, id):
        super(Player, self).__init__()
        self.surf = pygame.Surface((75, 25))
        self.surf.fill((255, 255, 255))
        self.rect = self.surf.get_rect()
        self.id = id

    def update(self, command):
        if command == "u":
            self.rect.move_ip(0, -5)
        if command == "d":
            self.rect.move_ip(0, 5)
        if command == "l":
            self.rect.move_ip(-5, 0)
        if command == "r":
            self.rect.move_ip(5, 0)

        if self.rect.left < 0:
            self.rect.left = 0
        if self.rect.right > SCREEN_WIDTH:
            self.rect.right = SCREEN_WIDTH
        if self.rect.top <= 0:
            self.rect.top = 0
        if self.rect.bottom >= SCREEN_HEIGHT:
            self.rect.bottom = SCREEN_HEIGHT

pygame.init()

SCREEN_WIDTH = 800
SCREEN_HEIGHT = 400

clock = pygame.time.Clock()
screen = pygame.display.set_mode((SCREEN_WIDTH, SCREEN_HEIGHT))
players = pygame.sprite.Group()

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print ("Socket successfully created")
except socket.error as err:
    print ("socket creation failed with error %s" %(err))

port = 6000
s.connect(('127.0.0.1', port))
id = int(s.recv(1).decode('utf-8'))
players.add(Player(id))

while True:
    p_id = s.recv(1).decode('utf-8')

    if p_id == "d":
        break

    players.add(Player(int(p_id)))

s_manager = selectors.DefaultSelector()
s_manager.register(s, selectors.EVENT_READ)

start_time = time.time()

while True:

    for event in pygame.event.get():
        # Check for KEYDOWN event
        if event.type == KEYDOWN:
            # If the Esc key is pressed, then exit the main loop
            if event.key == K_ESCAPE:
                break
        # Check for QUIT event. If QUIT, then set running to false.
        elif event.type == QUIT:
            break

    events = s_manager.select(0)

    if events != []:
        #handle events
        key, data = events[0]
        bufer = key.fileobj.recv(2).decode('utf-8')

        print(bufer)
        # if ()
        #new player entered the game
        if (len(bufer) == 1):
            players.add(Player(int(bufer)))

        else:
            for player in players:
                if(player.id == int(bufer[0])):
                    player.update(bufer[1])
                    break

    pressed_keys = pygame.key.get_pressed()

    if pressed_keys[K_UP]:
        s.send((str(id)+"u").encode())
    if pressed_keys[K_DOWN]:
        s.send((str(id)+"d").encode())
    if pressed_keys[K_LEFT]:
        s.send((str(id)+"l").encode())
    if pressed_keys[K_RIGHT]:
        s.send((str(id)+"r").encode())

    screen.fill((0, 0, 0))
    for player in players:
        screen.blit(player.surf, player.rect)

    pygame.display.flip()
    clock.tick(30)

pygame.quit()
