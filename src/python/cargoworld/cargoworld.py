
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





import sys, time, uuid, random, threading
from PyQt5.QtWidgets import QApplication
from PyQt5.QtCore import *

import cargoworld_quadtree
import cargoworld_interface
import cargoworld_gui
import cargoworld_telemetry

# DEFINITIONS ----------------------------------------------------------------------

CARGOWORLD_VERSION = '0.2'
CARGOWORLD_VERSION_NAME = 'Los Pollos Hermanos'

CARGOWORLD_LOCATION_DISTANCE_DETECTION = 100
CARGOWORLD_AUTOMATA_LOOP_DELAY = 2



# SIMULATOR ------------------------------------------------------------------------

class Simulator(QObject):
    'Simulator'

    telemetrysignal = pyqtSignal()
    
    def __init__(self, id, name, logo, world):
        super(Simulator, self).__init__()
        self.id = id
        self.name = name
        self.logo = logo
        self.world = world
        self.host = "localhost"
        self.telemetry = None
        self.telemetrythread = None
        self.button = None
        self.speedmultiplier = 1
        self.speedwarning = 0
        self.speedover = 0

    def startTelemetry(self):
        self.telemetry = cargoworld_telemetry.SCS(self.world, self.host)
        self.telemetry.handler = self.telemetryHandler
        self.telemetry.connectToSimulator()
        self.telemetrythread = threading.Thread(target = self.telemetry.processTelemetry)
        self.telemetrythread.start()

    def telemetryHandler(self):
        self.telemetrysignal.emit()

            
    def terminateTelemetry(self):
        if self.telemetry is not None:
            self.telemetry.terminateTelemetry()
            self.telemetry.closeSockets()
             
                
    def display(self):
        print("Simulator", self.id, self.name)
        

# PLAYER --------------------------------------------------------------------------

class Player:
    'Player'

    def __init__(self, id, name):
        self.id = id
        self.name = name
        self.companylogo = None
        self.showtrailer = True
        self.simulator = None
        self.location = Location(name)
        self.closelocation = None
        self.cargoarea = None
        self.cargodelivered = set()
        
    def display(self):
        print("Player", self.id, self.name, "Simulator", self.simulator.name,
              "Location", self.location.name)


# CARGOAREA------------------------------------------------------------------------

class CargoareaType:
    'CargoareaType'

    def __init__(self, type, nbslots, picture):
        self.type = type
        self.nbslots = nbslots
        if picture is None:
            self.picture = 'trailer_curtain.png'
        else:
            self.picture = picture
        

class Cargoarea:
    'Area to store cargo'

    def __init__(self, type):
        self.type = type
        self.area = [None]*type.nbslots


    def getAvailableSlot(self, nbslots):
        i = 0
        rem = nbslots
        slot = -1
        candidate = False
        while i < self.type.nbslots and rem > 0:
            if self.area[i] is None:
                if candidate is False:
                    slot = i
                    candidate = True
                rem -= 1    
            else:
                rem = nbslots
                slot = -1
                candidate = False
                    
            i += 1
            
        if rem > 0:
            slot = -1
            
        return slot

    def addCargoToArea(self, cargo, slot):
        for i in range(slot, slot + cargo.type.nbslots):
            self.area[i] = cargo

    def removeCargoFromArea(self, cargo, player):
        player.cargodelivered.add(cargo)
        for i in range(0, self.type.nbslots):
            if self.area[i] is not None and self.area[i].id == cargo.id:
                self.area[i] = None

            

# LOCATION ------------------------------------------------------------------------


class LocationType:
    'LocationType'
    
    def __init__(self, type, inputcargo, outputcargo):
        self.type = type
        self.inputcargo = inputcargo
        self.outputcargo = outputcargo

    def isCargoInput(self, cargotype):
        found = False
        if self.inputcargo is not None:
            i = 0
            while found == False and i < len(self.inputcargo):
                if self.inputcargo[i] == cargotype:
                    found = True
                i += 1
        return found
        
        


class Location:
    'Location'
    
    def __init__(self, name, type = None):
        self.name = name
        self.type = type
        self.positions = {}
        self.cargos = set()
        self.distance = float("inf")

    def addPosition(self, simulator, coordinates):
        self.positions[simulator.id] = coordinates

    def getPosition(self, simulator):
        if simulator.id in self.positions:
            return self.positions[simulator.id]
        else:
            return None
        
    def removePosition(self, simulator):
        if simulator.id in self.positions:
            del self.positions[simulator.id]

    def addCargo(self, cargo):
        self.cargos.add(cargo)
            
    def removeCargo(self, cargo):
        if cargo in self.cargos:
            self.cargos.remove(cargo)

    def getDistance(self, location):
        distance = float("inf")
        if location is not None:
            locset = set(location.positions)
            selfset = set(self.positions)
            for simtag in locset.intersection(selfset):
                locpos = location.positions[simtag]
                selfpos = self.positions[simtag]
                distance = abs(locpos[0] - selfpos[0]) + abs(locpos[1] - selfpos[1]) + abs(locpos[2] - selfpos[2])
        return distance

    def isInRange(self, location, cargorange):
        inrange = True
        thisnamelist = self.name.split(':')
        othernamelist = location.name.split(':')
        if len(thisnamelist) > 2 and len(othernamelist) > 2:
            if cargorange != "world":
                inrange = thisnamelist[0] == othernamelist[0]
                if cargorange == "city":
                    inrange = inrange and (thisnamelist[1] == othernamelist[1])

        return inrange


        
            
    def display(self):
        print("Location", self.name, "Positions", self.positions)



# CARGO ------------------------------------------------------------------------

class CargoType:
    'CargoType'

    def __init__(self, type, picture, cargoareatypes, nbslots, names, cargorange):
        self.type = type
        if picture is None:
            self.picture = 'cargo_default.png'
        else:
            self.picture = picture
        self.cargoareatypes = cargoareatypes
        self.nbslots = nbslots
        self.names = names
        self.cargorange = cargorange

        


class Cargo:
    'Cargo'

    def __init__(self, name, type, locfrom, locto):
        self.id = uuid.uuid4()
        self.name = name
        self.type = type
        self.location = locfrom
        self.destination = locto
        self.areaslot = -1
        self.cargowidget = None
        
    def setLocation(self, location):
        self.location = location
        location.addCargo(self)

    def setDestination(self, location):
        self.destination = location
        
    def display(self):
        print("Cargo", self.id, self.name, self.type)



# WORLD ------------------------------------------------------------------------

class World(QObject):
    'World'

    newcargosignal = pyqtSignal()
    telemetrysignal = pyqtSignal()
    positionsignal = pyqtSignal()
    messagesignal = pyqtSignal()
    cargoareasignal = pyqtSignal()

    version = CARGOWORLD_VERSION
    
    def __init__(self, name, app):
        super(World, self).__init__()

        self.lock = threading.Lock()

        self.app = app
        self.name = name
        self.interface = cargoworld_interface.WorldInterface()
        self.simulators = set()
        self.simulator = None
        self.locationtypes = set()
        self.locations = []
        self.locationstree = None
        self.cargotypes = {}
        self.cargostree = None
        self.cargoareamaxslots = 4
        self.cargoareatypes = {}
        self.trailertypes = {}
        self.players = set()
        self.player = None
        self.window = None
        self.state = 'INIT'

        self.interface.loadSimulators(self)
        self.interface.loadTypes(self)
        self.interface.loadPositions(self)
        self.interface.configRead(self)
        
        
    def chooseSimulator(self, window):
        self.window = window
        if self.window is not None:
            i = 0
            for sim in self.simulators:
                self.window.addSimulator(sim, 10 + i)
                i += 310

    def setSimulator(self, simulator):
        self.simulator = simulator
                

    def addPlayer(self, name, simulator):
        player = Player(0, name)
        player.simulator = simulator
        self.player = player
        self.players.add(player)
        self.interface.positionsQuadTree(self)
        
    def getPlayerPosition(self):
        pos = None
        if self.player is not None and self.simulator is not None:
            if self.player.location is not None and self.locationstree is not None:
                self.lock.acquire()
                position = self.player.location.getPosition(self.simulator)
                self.lock.release()
                if position is not None:
                    pos = position
        return pos

    def updateCargoarea(self):
        if self.simulator.telemetry is not None:
            cargoareatypestr = ""
            self.simulator.telemetry.lock.acquire()
            if self.simulator.telemetry.trailer in self.trailertypes:
                cargoareatypestr = self.trailertypes[self.simulator.telemetry.trailer]
            self.simulator.telemetry.lock.release()
            change = False
            if self.player.cargoarea is not None:
                if self.player.cargoarea.type.type != cargoareatypestr:
                    change = True
            else:
                change = True
                    
            if change:
                cargoareatype = None
                self.lock.acquire()
                if cargoareatypestr in self.cargoareatypes:
                    cargoareatype = self.cargoareatypes[cargoareatypestr]
                if cargoareatype is not None:
                    self.player.cargoarea = Cargoarea(cargoareatype)
                else:
                    self.player.cargoarea = None
                self.lock.release()
                self.cargoareasignal.emit()
            
    
    def updateClosestLocation(self):
        if self.simulator.telemetry is not None:
            self.simulator.telemetry.lock.acquire()
            x = self.simulator.telemetry.x
            y = self.simulator.telemetry.y
            z = self.simulator.telemetry.z
            self.simulator.telemetry.lock.release()
            self.lock.acquire()
            self.player.location.addPosition(self.simulator, [x, y , z])
            self.lock.release()
            distmin = float("inf")
            closeloc = None
            if self.locationstree is not None:
                loclist = self.locationstree.getNeighborhood(x, y)
                for location in loclist:
                    dist = location.getDistance(self.player.location)
                    if dist < distmin:
                        distmin = dist
                        closeloc = location
                if closeloc is not None:
                    if closeloc.getDistance(self.player.location) < CARGOWORLD_LOCATION_DISTANCE_DETECTION:
                        self.lock.acquire()
                        self.player.closelocation = closeloc
                        self.lock.release()
                    else:
                        self.lock.acquire()
                        self.player.closelocation = None
                        self.lock.release()

                
    
    def generateCargo(self):
        if self.player is not None and self.simulator is not None:
            if self.player.location is not None and self.locationstree is not None:
                position = self.player.location.getPosition(self.simulator)
                if position is not None:
                    x = position[0]
                    y = position[1]
                    locations = self.locationstree.getNeighborhood(x, y)
                    random.shuffle(locations)
                    locfrom = None
                    i = 0
                    while locfrom is None and i < len(locations):
                        location = locations[i]
                        if location.type is not None:
                            if location.type.outputcargo is not None:
                                locfrom = location
                        i += 1
                        
                    if locfrom is not None:
                        cargotypeid = locfrom.type.outputcargo[random.randint(0, len(locfrom.type.outputcargo) - 1)]
                        cargotype = self.cargotypes[cargotypeid]
                        if cargotype is not None:
                            random.shuffle(self.locations)
                            locto = None
                            i = 0
                            while locto is None and i < len(self.locations):
                                location = self.locations[i]
                                if locfrom.isInRange(location, cargotype.cargorange):
                                    if location.type is not None:
                                        if location.type.isCargoInput(cargotype.type):
                                            # disables inter-simulator cargos
                                            if location.getPosition(self.simulator) is not None:
                                                locto = location
                                i += 1
                            if locto is not None and locto is not locfrom:
                                name = cargotype.names[random.randint(0,
                                                                      len(cargotype.names) - 1)] 
                                cargo = Cargo(name, cargotype, locfrom, locto)
                                position = locfrom.positions[world.simulator.id]
                                x = position[0]
                                y = position[1]
                                self.lock.acquire()
                                self.cargostree.add(cargo, x, y)
                                self.lock.release()
                                self.newcargosignal.emit()
                                self.interface.cargograph.addCargo(cargo, self.player.closelocation)
                                print(locfrom.name, '->', locto.name, ':',
                                      cargotype.type, 'range', cargotype.cargorange)


    def removeCargoFromArea(self, cargo):
        self.lock.acquire()
        if cargo.destination is self.player.closelocation:
            cargo.destination.removeCargo(cargo)
            self.cargostree.removeRecursive(cargo)
        self.lock.release()


                                
    def start(self):
        self.state = 'PAUSED'
        self.automata()

    def terminate(self):
        self.state = 'TERMINATE'
        if self.interface is not None:
            # if self.interface.which("dot") is not None:
            #     self.interface.cargograph.world(self)
            if self.player is not None:
                self.interface.dumpDeliveredCargo(self.player)
        if self.simulator is not None:
            self.simulator.terminateTelemetry()
        
    def automata(self):
        curstate = 'PAUSED'
        while curstate is 'RUNNING' or curstate is 'PAUSED':
            self.updateCargoarea()
            self.updateClosestLocation()
            self.telemetrysignal.emit()
            self.positionsignal.emit()
            self.lock.acquire()
            curstate = self.state
            self.lock.release()
            if curstate is 'RUNNING':
                self.generateCargo()
            time.sleep(CARGOWORLD_AUTOMATA_LOOP_DELAY)

        
    def display(self):
        print("World", self.name)
        
                

# MAIN ------------------------------------------------------------------------

if __name__ == '__main__':

    print('CargoWorld ', CARGOWORLD_VERSION)
    
    app = QApplication(sys.argv)
        
    world = World("CargoWorld", app)
    
    window = cargoworld_gui.MainWindow(world)

    world.chooseSimulator(window)
   
    sys.exit(app.exec_())
