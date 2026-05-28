import app
from app_components import clear_background, Menu
from app_components.tokens import colors
from events.input import Buttons, BUTTON_TYPES, ButtonDownEvent
from system.eventbus import eventbus
from system.hexpansion.config import HexpansionConfig
from system.scheduler.events import RequestForegroundPushEvent
from system.patterndisplay.events import PatternDisable, PatternEnable
import machine
import time
import struct

RADIOLARIAN_I2C_ADDRESS = 0x62
RADIOLARIAN_CMD_READ_MSG_LENGTH = 1
RADIOLARIAN_CMD_READ_MSG_BODY = 2
RADIOLARIAN_CMD_READ_RIC = 3
RADIOLARIAN_CMD_READ_FREQUENCY = 4
RADIOLARIAN_CMD_READ_BAUD = 5
RADIOLARIAN_CMD_MSG_RECEIVED = 0x10
RADIOLARIAN_CMD_NEXT_SETTING = 0x11

class BuzzbyApp(app.App):
    def __init__(self, config: HexpansionConfig):
        print("in __init__")
        self.hexp_config = config
        self.i2c = self.hexp_config.i2c
        eventbus.on(ButtonDownEvent, self._handle_buttondown, self)
        self.pins = {}
        self.pins["ls_1"] = self.hexp_config.ls_pin[0]
        self.pins["ls_2"] = self.hexp_config.ls_pin[1]
        self.pins["ls_3"] = self.hexp_config.ls_pin[2]
        self.pins["ls_4"] = self.hexp_config.ls_pin[3]
        self.pins["ls_5"] = self.hexp_config.ls_pin[4]
        self.pins["hs_1"] = self.hexp_config.pin[0]
        self.pins["hs_2"] = self.hexp_config.pin[1]
        self.pins["hs_3"] = self.hexp_config.pin[2]
        self.pins["hs_4"] = self.hexp_config.pin[3]
        self.foregrounded = False
        self.fetchingConfig = True
        self.commandSent = 0
        self.cmdSentAt = 0
        self.messageLength = 0
        self.lastmessage = "Hello from Buzzby on Radiolarian! Waiting for messages..."
        self.ric = ""
        self.text_width = 0
        self.textX = 200
        self.scrollStart = 0
        self.frequency = 0
        self.baud = 0
        print(self.i2c.scan())
        self.pins["hs_1"].init(self.pins["hs_1"].IN)
        #self.pins["hs_1"].irq(
        #    handler=self._handle_pagermessagerx,
        #    trigger=self.pins["hs_1"].IRQ_RISING,
        #    wake=(1 | machine.SLEEP | machine.DEEPSLEEP)
        #)

    def background_update(self, delta_ticks):
        self.currentI2CDevices = self.i2c.scan()
        if RADIOLARIAN_I2C_ADDRESS in self.currentI2CDevices:
            self.radiolarianConnected = True
        else:
            self.radiolarianConnected = False

        if self.text_width > 200:
            advance = (time.ticks_ms() - self.scrollStart) / 10
            self.textX = 200 - advance
            if (self.textX + self.text_width) < -200:
                self.textX = -200 - self.text_width
                if advance > 350:
                    self.textX = 200
                    self.scrollStart = time.ticks_ms()
        else:
            self.textX = -100

        if self.commandSent > 0 and (self.cmdSentAt + 100) > time.ticks_ms():
            # A command has been sent, but give it time before we try following up
            return
        if self.pins["hs_1"].value() == 1 and self.fetchingConfig == False:
            if self.commandSent == 0:
                print("Sending RADIOLARIAN_CMD_READ_MSG_LENGTH")
                self.messageLength = 0
                self.i2c.writeto(RADIOLARIAN_I2C_ADDRESS, bytearray([RADIOLARIAN_CMD_READ_MSG_LENGTH]), True)
                self.commandSent = RADIOLARIAN_CMD_READ_MSG_LENGTH
                self.cmdSentAt = time.ticks_ms()
                return
            if self.commandSent == RADIOLARIAN_CMD_READ_MSG_LENGTH and self.messageLength == 0:
                print("Asking for message length after sending RADIOLARIAN_CMD_READ_MSG_LENGTH")
                self.messageLength = self.read_byte_from_radiolarian()
                print(f"  Got length of {self.messageLength}")
                if self.messageLength < 1:
                    self.commandSent = 0
                return
            if self.commandSent == RADIOLARIAN_CMD_READ_MSG_LENGTH and self.messageLength > 0:
                print("Sending RADIOLARIAN_CMD_READ_RIC")
                self.ric = ""
                self.i2c.writeto(RADIOLARIAN_I2C_ADDRESS, bytearray([RADIOLARIAN_CMD_READ_RIC]))
                self.commandSent = RADIOLARIAN_CMD_READ_RIC
                self.cmdSentAt = time.ticks_ms()
                return
            if self.commandSent == RADIOLARIAN_CMD_READ_RIC and self.ric == "":
                print("Asking for RIC follwoing sending RADIOLARIAN_CMD_READ_RIC")
                self.ric = str(self.read_uint32_from_radiolarian())
                print(f"  Got RIC of {self.ric}")
                return
            if self.commandSent == RADIOLARIAN_CMD_READ_RIC and len(self.ric) > 0:
                print("Sending RADIOLARIAN_CMD_READ_MSG_BODY")
                self.i2c.writeto(RADIOLARIAN_I2C_ADDRESS, bytearray([RADIOLARIAN_CMD_READ_MSG_BODY]))
                self.commandSent = RADIOLARIAN_CMD_READ_MSG_BODY
                self.cmdSentAt = time.ticks_ms()
                return
            if self.commandSent == RADIOLARIAN_CMD_READ_MSG_BODY:
                print(f"Asking for message body with length {self.messageLength} after sending RADIOLARIAN_CMD_READ_MSG_BODY")
                self.lastmessage = self.read_string_from_radiolarian(self.messageLength)
                self.textX = 200
                self.scrollStart = time.ticks_ms()
                print("Sending RADIOLARIAN_CMD_MSG_RECEIVED")
                self.i2c.writeto(RADIOLARIAN_I2C_ADDRESS, bytearray([RADIOLARIAN_CMD_MSG_RECEIVED]))
                self.commandSent = RADIOLARIAN_CMD_MSG_RECEIVED
                self.cmdSentAt = time.ticks_ms()
                return
            if self.commandSent == RADIOLARIAN_CMD_MSG_RECEIVED:
                print("Clearing command having sent RADIOLARIAN_CMD_MSG_RECEIVED")
                self.commandSent = 0;
                return
        elif self.fetchingConfig:
            if (self.commandSent == RADIOLARIAN_CMD_NEXT_SETTING
                    or (self.frequency == 0 and self.commandSent == 0)):
                print("Sending RADIOLARIAN_CMD_READ_FREQUENCY")
                self.i2c.writeto(RADIOLARIAN_I2C_ADDRESS, bytearray([RADIOLARIAN_CMD_READ_FREQUENCY]))
                self.commandSent = RADIOLARIAN_CMD_READ_FREQUENCY
                self.cmdSentAt = time.ticks_ms()
                return
            if self.commandSent == RADIOLARIAN_CMD_READ_FREQUENCY:
                print("Asking for frequency having sent RADIOLARIAN_CMD_READ_FREQUENCY")
                self.frequency = self.read_uint32_from_radiolarian()
                print(f"  Got frequency of {self.frequency}")
                print("Sending RADIOLARIAN_CMD_READ_BAUD")
                self.i2c.writeto(RADIOLARIAN_I2C_ADDRESS, bytearray([RADIOLARIAN_CMD_READ_BAUD]))
                self.commandSent = RADIOLARIAN_CMD_READ_BAUD
                self.cmdSentAt = time.ticks_ms()
                return
            if self.commandSent == RADIOLARIAN_CMD_READ_BAUD:
                print("Asking for bitrate having sent RADIOLARIAN_CMD_READ_BAUD")
                self.baud = self.read_uint32_from_radiolarian()
                print(f"  Got bitrate of {self.baud}")
                self.commandSent = 0
                self.fetchingConfig = False
                return


    def read_string_from_radiolarian(self, num_bytes=256, timeout_ms=500):
        t0 = time.ticks_ms()
        while True:
            try:
                print("read_string_from_radiolarian()", num_bytes)
                data = self.i2c.readfrom(RADIOLARIAN_I2C_ADDRESS, num_bytes)
                data = bytes(b for b in data if 32 <= b <= 126 or b in (9,10,13))
                print(f"  read_string_from_radiolarian() got string: {data.decode('utf-8', 'ignore')}")
                return data.decode('utf-8', 'ignore')
            except OSError:
                print("  read_string_from_radiolarian() OSError")
                if time.ticks_diff(time.ticks_ms(), t0) > timeout_ms:
                    print(f"  Timed out trying to read string from hexpansion after command {self.commandSent}")
                time.sleep_ms(10)

    def read_uint32_from_radiolarian(self):
        t0 = time.ticks_ms()
        while True:
            try:
                print("read_uint32_from_radiolarian()")
                data = self.i2c.readfrom(RADIOLARIAN_I2C_ADDRESS, 4)
                ricky = struct.unpack('<I', data)[0]
                print(f"  read_uint32_from_radiolarian() got uint32 {ricky}")
                return ricky
            except OSError:
                print("  read_uint32_from_radiolarian() OSError")
                if time.ticks_diff(time.ticks_ms(), t0) > 500:
                    print(f"  Timed out trying to read uint32 from hexpansion after command {self.commandSent}")
                time.sleep_ms(10)

    def read_byte_from_radiolarian(self):
        t0 = time.ticks_ms()
        while True:
            try:
                print("read_byte_from_radiolarian()")
                data = self.i2c.readfrom(RADIOLARIAN_I2C_ADDRESS, 1)[0]
                print(f"  read_byte_from_radiolarian() got byte {int(data)}")
                return int(data)
            except OSError:
                print("  read_byte_from_radiolarian() OSError")
                if time.ticks_diff(time.ticks_ms(), t0) > 500:
                    print(f"  Timed out trying to read byte from hexpansion after command {self.commandSent}")
                time.sleep_ms(10)


    def update(self, delta):
        if not self.foregrounded: # Bring the app to the foreground on first run
            eventbus.emit(RequestForegroundPushEvent(self))
            self.foregrounded = True

    def _handle_buttondown(self, event: ButtonDownEvent):
        if BUTTON_TYPES["UP"] in event.button:
            message_waiting = self.pins["hs_1"].value()
            print(f"up button pressed - message waiting: {message_waiting} fetching config: {self.fetchingConfig}")
            if ((message_waiting == 0) and (not self.fetchingConfig)):
                print("Message not waiting and not trying to fetch config... trying to switch channel")
                self.fetchingConfig = True
                self.i2c.writeto(RADIOLARIAN_I2C_ADDRESS, bytearray([RADIOLARIAN_CMD_NEXT_SETTING]))
                self.cmdSentAt = time.ticks_ms()

    def draw(self, ctx):
        ctx.save()
        clear_background(ctx)
        ctx.text_align = ctx.LEFT
        ctx.font_size = 26
        if (self.baud > 0):
            ctx.rgb(.4,.4,.4).move_to(-60,-85).text(f"{self.baud} baud")
        if (self.frequency > 0):
            ctx.rgb(.8,0,.8).move_to(-80,-60).text(f"{(self.frequency/1000000):8.3f} MHz")
        if len(self.ric) > 0:
            ctx.rgb(0,.8,.8).move_to(-70,-30).text(f"RIC:{self.ric}")
        ctx.font_size = 32
        self.text_width = ctx.text_width(self.lastmessage)
        ctx.rgb(0, 1, 0).move_to(self.textX, 0).text(self.lastmessage)
        ctx.restore()

    def _cleanup(self):
        print("in cleanup")
        eventbus.remove(ButtonDownEvent, self._handle_buttondown, self.app)

__app_export__ = BuzzbyApp
