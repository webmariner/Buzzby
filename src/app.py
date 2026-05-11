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
RADIOLARIAN_CMD_READ_SETTING = 4
RADIOLARIAN_CMD_MSG_RECEIVED = 0x10
RADIOLARIAN_CMD_NEXT_SETTING = 0x11

class BuzzbyApp(app.App):
    def __init__(self, config: HexpansionConfig):
        print("in __init__")
        self.hexp_config = config
        self.i2c = self.hexp_config.i2c
        self.button_states = Buttons(self)
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
        self.commandSent = 0
        self.cmdSentAt = 0
        self.messageLength = 0
        self.lastmessage = "Hello from Buzzby on Radiolarian! Waiting for messages..."
        self.ric = ""
        self.text_width = 0
        self.textX = 200
        self.scrollStart = 0
        print(self.i2c.scan())
        self.pins["hs_1"].init(self.pins["hs_1"].IN)
        #self.pins["hs_1"].irq(
        #    handler=self._handle_pagermessagerx,
        #    trigger=self.pins["hs_1"].IRQ_RISING,
        #    wake=(1 | machine.SLEEP | machine.DEEPSLEEP)
        #)

    def background_update(self, delta_ticks):
        # self.currentI2CDevices = self.i2c.scan()
        # if RADIOLARIAN_I2C_ADDRESS in self.currentI2CDevices:
        #     self.radiolarianConnected = True
        # else:
        #     self.radiolarianConnected = False

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


        if self.pins["hs_1"].value() == 1:
            if self.commandSent == 0 or self.commandSent == RADIOLARIAN_CMD_MSG_RECEIVED: # or any other command not in the message fetching chain below...
                if self.commandSent > 0 and (self.cmdSentAt + 1200) > time.ticks_ms():
                    # A command has been sent, but give it time before we try asking for data
                    return
                self.messageLength = 0
                self.i2c.writeto(RADIOLARIAN_I2C_ADDRESS, bytearray([RADIOLARIAN_CMD_READ_MSG_LENGTH]), True)
                self.commandSent = RADIOLARIAN_CMD_READ_MSG_LENGTH
                self.cmdSentAt = time.ticks_ms()
                return
            if self.commandSent == RADIOLARIAN_CMD_READ_MSG_LENGTH and self.messageLength == 0:
                self.messageLength = self.read_byte_from_radiolarian()
                if self.messageLength < 1:
                    self.commandSent = 0
                return
            if self.commandSent == RADIOLARIAN_CMD_READ_MSG_LENGTH and self.messageLength > 0:
                self.ric = ""
                self.i2c.writeto(RADIOLARIAN_I2C_ADDRESS, bytearray([RADIOLARIAN_CMD_READ_RIC]))
                self.commandSent = RADIOLARIAN_CMD_READ_RIC
                self.cmdSentAt = time.ticks_ms()
                return
            if self.commandSent == RADIOLARIAN_CMD_READ_RIC and self.ric == "":
                self.ric = str(self.read_uint32_from_radiolarian())
                return
            if self.commandSent == RADIOLARIAN_CMD_READ_RIC and len(self.ric) > 0:
                self.i2c.writeto(RADIOLARIAN_I2C_ADDRESS, bytearray([RADIOLARIAN_CMD_READ_MSG_BODY]))
                self.commandSent = RADIOLARIAN_CMD_READ_MSG_BODY
                self.cmdSentAt = time.ticks_ms()
                return
            if self.commandSent == RADIOLARIAN_CMD_READ_MSG_BODY:
                self.lastmessage = self.read_string_from_radiolarian(self.messageLength)
                self.textX = 200
                self.scrollStart = time.ticks_ms()
                self.i2c.writeto(RADIOLARIAN_I2C_ADDRESS, bytearray([RADIOLARIAN_CMD_MSG_RECEIVED]))
                self.commandSent = RADIOLARIAN_CMD_MSG_RECEIVED
                self.cmdSentAt = time.ticks_ms()
        else:
            if self.


    def read_string_from_radiolarian(self, num_bytes=256, timeout_ms=500):
        t0 = time.ticks_ms()
        while True:
            try:
                data = self.i2c.readfrom(RADIOLARIAN_I2C_ADDRESS, num_bytes)
                data = bytes(b for b in data if 32 <= b <= 126 or b in (9,10,13))
                return data.decode('utf-8', 'ignore')
            except OSError:
                if time.ticks_diff(time.ticks_ms(), t0) > timeout_ms:
                    print(f"Timed out trying to read string from hexpansion after command {self.commandSent}")
                time.sleep_ms(10)

    def read_uint32_from_radiolarian(self):
        t0 = time.ticks_ms()
        while True:
            try:
                data = self.i2c.readfrom(RADIOLARIAN_I2C_ADDRESS, 4)
                ricky = struct.unpack('<I', data)[0]
                return ricky
            except OSError:
                if time.ticks_diff(time.ticks_ms(), t0) > 500:
                    print(f"Timed out trying to read uint32 from hexpansion after command {self.commandSent}")
                time.sleep_ms(10)

    def read_byte_from_radiolarian(self):
        t0 = time.ticks_ms()
        while True:
            try:
                data = self.i2c.readfrom(RADIOLARIAN_I2C_ADDRESS, 1)[0]
                return int(data)
            except OSError:
                if time.ticks_diff(time.ticks_ms(), t0) > 500:
                    print(f"Timed out trying to read byte from hexpansion after command {self.commandSent}")
                time.sleep_ms(10)


    def update(self, delta):
        if not self.foregrounded: # Bring the app to the foreground on first run
            eventbus.emit(RequestForegroundPushEvent(self))
            self.foregrounded = True
    
    #def _handle_pagermessagerx(self, epin):
        #print("pagermessagerx irq handler called")
        #print(self.pins["hs_1"].value())
        #if not self.messageWaiting:
            #self.commandBytesSent = 0
            #self.messageWaiting = True

    def draw(self, ctx):
        ctx.save()
        clear_background(ctx)
        if len(self.ric) > 0:
            ctx.font_size = 26
            ctx.text_align = ctx.LEFT
            ctx.rgb(.4,.4,.4).move_to(-80,-30).text(f"RIC:{self.ric}")
        ctx.font_size = 32
        ctx.text_align = ctx.LEFT
        self.text_width = ctx.text_width(self.lastmessage)
        ctx.rgb(0, 1, 0).move_to(self.textX, 0).text(self.lastmessage)
        ctx.restore()

    def _cleanup(self):
        print("in cleanup")
        eventbus.remove(ButtonDownEvent, self._handle_buttondown, self.app)

__app_export__ = BuzzbyApp
