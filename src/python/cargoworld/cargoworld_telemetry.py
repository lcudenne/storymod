

# /**************************************************************************************
#  * CARGOWORLD
#  *
#  * Website: https://sites.google.com/site/storymodsite
#  * GitHub: https://github.com/lcudenne/storymod
#  * Author: Loic Cudennec <loic@cudennec.fr>
#  *
#  **************************************************************************************
#  *
#  * This program is free software: you can redistribute it and/or modify
#  * it under the terms of the GNU General Public License as published by
#  * the Free Software Foundation, either version 3 of the License, or
#  * (at your option) any later version.
#  *
#  * This program is distributed in the hope that it will be useful,
#  * but WITHOUT ANY WARRANTY; without even the implied warranty of
#  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  * GNU General Public License for more details.
#  *
#  * You should have received a copy of the GNU General Public License
#  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
#  * 
#  **************************************************************************************/



import socket
import sys
import threading
import time


CLIENT_PORT=35057
SERVER_PORT=35056

UDP_DATAGRAM_SIZE = 512

DATAGRAM_TYPE_SHUTDOWN = -1
DATAGRAM_TYPE_POSITION = 0
DATAGRAM_TYPE_CARGO = 1
DATAGRAM_TYPE_LBLINKER = 2
DATAGRAM_TYPE_RBLINKER = 3
DATAGRAM_TYPE_TRAILER_CONNECTED = 4
DATAGRAM_TYPE_CLIENT_VERSION = 5
DATAGRAM_TYPE_ENGINE_ENABLED = 6
DATAGRAM_TYPE_PARKING_BRAKE = 7
DATAGRAM_TYPE_TRAILER = 8


class SCS:
    'SCS simulators interface'

    def __init__(self, world, clienturl = "localhost", serverport = SERVER_PORT):

        self.world = world
        
        self.handler = None

        self.telelog = False
        self.telelogfile = None
        
        self.clienturl = clienturl
        self.clientadr = (clienturl, CLIENT_PORT)
        self.serverurl = self.getIpAddress()
        self.serverport = serverport
        self.recvsocket = None
        self.sendsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

        self.lock = threading.Lock()

        self.clientversionmaj = 0
        self.clientversionmin = 0

        self.x = 0
        self.y = 0
        self.z = 0
        self.speed = 0

        self.cargoid = ""
        self.cargoname = ""
        self.lblinker = 0
        self.rblinker = 0
        self.trailerconnected = 0
        self.trailertype = ""
        self.engineenabled = 0
        self.parkingbrake = 0
        self.trailer = ""



        
        
    def getIpAddress(self):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        try:
            s.connect(("8.8.8.8", 80))
            res = s.getsockname()[0]
        except socket.error as msg:
            res = "127.0.0.1"
        return res

    def openTelelog(self):
        filename = time.strftime("driving_trace_%Y_%m_%d_%H_%M_%S")
        self.lock.acquire()
        if self.telelogfile is not None:
            self.telelogfile.close()
            self.telelogfile = None
        self.telelogfile = open(filename, "w")
        self.telelog = True
        self.lock.release()
        
    def closeTelelog(self):
        self.lock.acquire()
        self.telelog = False
        if self.telelogfile is not None:
            self.telelogfile.close()
            self.telelogfile = None       
        self.lock.release()


        
    def connectToSimulator(self):
        if self.recvsocket is None:
            self.recvsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            serveradr = (self.serverurl, self.serverport)
            self.recvsocket.bind(serveradr)
            
        registermsg = bytes(self.serverurl + " " + str(self.serverport), 'utf-8')
        self.sendsocket.sendto(registermsg, self.clientadr)
        self.world.window.message = "connecting to "+ str(self.clientadr)+ " with "+ str(registermsg)
        self.world.messagesignal.emit()


    def processDatagram(self, datagram):
        cont = True
        
        datagramlist = datagram.decode('utf-8').split(' ')
        type = int(datagramlist[0])

        self.lock.acquire()

        if self.telelog and self.telelogfile is not None:
            self.telelogfile.write(datagram.decode('utf-8') + "\n")
        
        if type == DATAGRAM_TYPE_CLIENT_VERSION:
            self.clientversionmaj = int(datagramlist[1])
            self.clientversionmin = int(datagramlist[2])
        if type == DATAGRAM_TYPE_POSITION:
            self.x = float(datagramlist[1])
            self.y = float(datagramlist[2])
            self.z = float(datagramlist[3])
            self.speed = float(datagramlist[4])
        if type == DATAGRAM_TYPE_CARGO:
            self.cargoid = datagramlist[1]
            self.cargoname = datagramlist[2]
        if type == DATAGRAM_TYPE_LBLINKER:
            self.lblinker = int(datagramlist[1])
        if type == DATAGRAM_TYPE_RBLINKER:
            self.rblinker = int(datagramlist[1])
        if type == DATAGRAM_TYPE_TRAILER_CONNECTED:
            self.trailerconnected = int(datagramlist[1])
        if type == DATAGRAM_TYPE_ENGINE_ENABLED:
            self.engineenabled = int(datagramlist[1])
        if type == DATAGRAM_TYPE_PARKING_BRAKE:
            self.parkingbrake = int(datagramlist[1])
        if type == DATAGRAM_TYPE_TRAILER:
            self.trailer = datagramlist[1]
        if type == DATAGRAM_TYPE_SHUTDOWN:
            cont = False
            
        self.lock.release()

        self.handler()
#        self.display()

        return cont
        
        
    def processTelemetry(self):
        cont = True
        while cont:
            data, client = self.recvsocket.recvfrom(UDP_DATAGRAM_SIZE)
            cont = self.processDatagram(data)
        self.closeTelelog()

    def terminateTelemetry(self):
        registermsg = bytes(str(DATAGRAM_TYPE_SHUTDOWN), 'utf-8')
        serveradr = (self.serverurl, self.serverport)
        self.sendsocket.sendto(registermsg, serveradr)


    def createDatagram(self, type, values):
        return bytes(str(type) + " " + values, 'utf-8')
    

    def clientPlaceholder(self, trace=None):
        self.recvsocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        serveradr = ('0.0.0.0', CLIENT_PORT)
        self.recvsocket.bind(serveradr)
        data, server = self.recvsocket.recvfrom(UDP_DATAGRAM_SIZE)
        tokenlist = data.decode('utf-8').split(' ')
        print("contact server", tokenlist, "from", server)
        serveradr = (tokenlist[0], int(tokenlist[1]))

        msg = self.createDatagram(DATAGRAM_TYPE_CLIENT_VERSION, "0 0")
        self.sendsocket.sendto(msg, serveradr)

        if trace is not None:
            with open(trace) as trf:
                for line in trf:
                    msg = bytes(line, 'utf-8')
                    self.sendsocket.sendto(msg, serveradr)
                    time.sleep(0.01)

        msg = bytes(str(DATAGRAM_TYPE_SHUTDOWN), 'utf-8')
        self.sendsocket.sendto(msg, serveradr)
                    

                    
    def closeSockets(self):
        self.sendsocket.close()
        self.recvsocket.close()

    def display(self):
        print("connected client", self.clientadr,
              "v", self.clientversionmaj, ".", self.clientversionmin,
              "position x", self.x, "y", self.y, "z", self.z, "speed", self.speed,
              "cargoid", self.cargoid, "cargoname", self.cargoname,
              "lblinker", self.lblinker, "rblinker", self.rblinker,
              "trailerconnected", self.trailerconnected, "engineenabled", self.engineenabled,
              "parkingbrake", self.parkingbrake)
        
        

# MAIN ------------------------------------------------------------------------


if __name__ == '__main__':

    scs = SCS(None)
        
    for i in range(len(sys.argv)):
        if sys.argv[i] == "--client":
            trace = None
            if (i + 1) < len(sys.argv):
                trace = sys.argv[i+1]
            scs.clientPlaceholder(trace)
        if sys.argv[i] == "--server":
            scs.connectToSimulator()
            scs.processTelemetry()
            
    scs.closeSockets()
