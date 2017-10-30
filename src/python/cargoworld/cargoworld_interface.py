
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





import configparser
import os
import glob
import sys
import math
import random

try:
    from lxml import etree
except ImportError:
    import xml.etree.ElementTree as etree

import cargoworld
import cargoworld_quadtree
import cargoworld_graphviz


# DEFINITIONS ----------------------------------------------------------------------

CARGOWORLD_CONFIGURATION_FILE = "config.ini"
CARGOWORLD_DATABASE_BASENAME = "cargoworld_database"

# INTERFACE ------------------------------------------------------------------------

class WorldInterface:
    'Interface'

    def __init__(self):
        self.config = None
        self.cargograph = cargoworld_graphviz.CargoGraph()


    def is_exe(self, fpath):
        return os.path.isfile(fpath) and os.access(fpath, os.X_OK)

    def which(self, program):
        fpath, fname = os.path.split(program)
        if fpath:
            if self.is_exe(program):
                return program
        else:
            for path in os.environ["PATH"].split(os.pathsep):
                path = path.strip('"')
                exe_file = os.path.join(path, program)
                if self.is_exe(exe_file):
                    return exe_file
                
        return None

        
    def configRead(self, world):
        if not os.path.isfile(CARGOWORLD_CONFIGURATION_FILE):
            self.configWrite(world)
        else:
            print("Reading", CARGOWORLD_CONFIGURATION_FILE)
            self.config = configparser.ConfigParser()
            self.config.read(CARGOWORLD_CONFIGURATION_FILE)
                
    def configWrite(self, world):
        if world.simulator is not None and world.player is not None:
            print("Writing", CARGOWORLD_CONFIGURATION_FILE)
            cfgfile = open(CARGOWORLD_CONFIGURATION_FILE, "w")
            if self.config is None:
                self.config = configparser.ConfigParser()
                self.config.add_section('ETS2')
            self.config.set('ETS2', 'nick', world.player.name)
            self.config.set('ETS2', 'host', world.simulator.telemetry.clienturl)
            self.config.write(cfgfile)
            cfgfile.close()
        
    
    def loadSimulators(self, world):
        sim = cargoworld.Simulator("ETS2",
                                    "SCS Software Euro Truck Simulator 2",
                                    "img/sim_ets2_logo.png",
                                    world)
        sim.speedmultiplier = 3.6
        sim.speedwarning = 80
        sim.speedover = 90
        world.simulators.add(sim)
        sim = cargoworld.Simulator("ATS",
                                    "SCS Software American Truck Simulator",
                                    "img/sim_ats_logo.png",
                                    world)
        sim.speedwarning = 55
        sim.speedover = 65
        world.simulators.add(sim)
        

    def loadTypes(self, world):
        nlocations = 0
        ncargoarea = 0
        ncargo = 0
        ntrailer = 0
        
        for file in glob.glob('.' + '/**/' + CARGOWORLD_DATABASE_BASENAME + '*.xml', recursive=True):
            print('Loading', file)

        
        
        for file in glob.glob('.' + '/**/' + CARGOWORLD_DATABASE_BASENAME + '*.xml', recursive=True):
            tree = etree.parse(file)
            root = tree.getroot()
            for locationnode in tree.findall('.//LOCATION_TYPE'):
                type = locationnode.get('type')
                inputcargo = None
                outputcargo = None
                inputcargostr = locationnode.get('inputcargo')
                if inputcargostr is not None:
                    inputcargo = inputcargostr.split(',')
                outputcargostr = locationnode.get('outputcargo')
                if outputcargostr is not None:
                    outputcargo = outputcargostr.split(',')
                locationtype = cargoworld.LocationType(type, inputcargo, outputcargo)
                world.locationtypes.add(locationtype)
                nlocations = nlocations + 1

                
        for file in glob.glob('.' + '/**/' + CARGOWORLD_DATABASE_BASENAME + '*.xml', recursive=True):
            tree = etree.parse(file)
            root = tree.getroot()            
            for cargoareanode in tree.findall('.//CARGOAREA_TYPE'):
                type = cargoareanode.get('type')
                nbslots = int(cargoareanode.get('nbslots'))
                picture = cargoareanode.get('picture')
                cargoareatype = cargoworld.CargoareaType(type, nbslots, picture)
                world.cargoareatypes[type] = cargoareatype
                ncargoarea = ncargoarea + 1

                
        for file in glob.glob('.' + '/**/' + CARGOWORLD_DATABASE_BASENAME + '*.xml', recursive=True):
            tree = etree.parse(file)
            root = tree.getroot()                
            for cargonode in tree.findall('.//CARGO_TYPE'):
                type = cargonode.get('type')
                picture = cargonode.get('picture')
                cargoareatypelist = cargonode.get('cargoarea').split(',')
                cargoareatypes = set()
                for cargoareatypestr in cargoareatypelist:
                    cargoareatype = world.cargoareatypes[cargoareatypestr]
                    if cargoareatype is not None:
                        cargoareatypes.add(cargoareatype)
                nbslots = int(cargonode.get('nbslots'))
                names = cargonode.get('names').split(',')
                cargorange = cargonode.get('range')
                cargotype = cargoworld.CargoType(type, picture, cargoareatypes, nbslots, names, cargorange)
                world.cargotypes[type] = cargotype
                ncargo = ncargo + 1

                
        for file in glob.glob('.' + '/**/' + CARGOWORLD_DATABASE_BASENAME + '*.xml', recursive=True):
            tree = etree.parse(file)
            root = tree.getroot()                
            for trailernode in tree.findall('.//TRAILER_TYPE'):
                type = trailernode.get('type')
                cargoarea = trailernode.get('cargoarea')
                world.trailertypes[type] = cargoarea
                ntrailer = ntrailer + 1

        print('Added types:', nlocations, 'locations', ncargoarea, 'cargoarea',
              ncargo, 'cargo', ntrailer, 'trailers')

                

    def addPosition(self, world, simulator, name, type, x, y, z):
        locationtype = None
        if type is not None:
            for typecandidate in world.locationtypes:
                if typecandidate.type == type:
                    locationtype = typecandidate
            if locationtype is None:
                print('Could not find location type', type)

        location = None
        for loccandidate in world.locations:
            if loccandidate.name == name:
                location = loccandidate

        returnlocation = None
        if location is None:
            location = cargoworld.Location(name, locationtype)
            returnlocation = location
            
        world.locations.append(location)
        location.addPosition(simulator, [x, y, z])
        
        return returnlocation
        

                
    def loadPositions(self, world):
        for file in glob.glob('.' + '/**/' + CARGOWORLD_DATABASE_BASENAME + '*.xml', recursive=True):
            tree = etree.parse(file)
            root = tree.getroot()
            positions = tree.find('.//POSITIONS')
            if positions is not None:
                simid = positions.get('simulator')
                simulator = None
                for simcandidate in world.simulators:
                    if simcandidate.id == simid:
                        simulator = simcandidate
                if simulator is not None:
                    for positionnode in tree.findall('.//POSITION'):
                        name = positionnode.get('name')
                        type = positionnode.get('type')

                        x = float(positionnode.get('x'))
                        y = float(positionnode.get('y'))
                        z = float(positionnode.get('z'))

                        self.addPosition(world, simulator, name, type, x, y, z)
                    
                else:
                    print('Could not find simulator', simid)

        if world.locations is not None:
            random.shuffle(world.locations)
            


    def positionsQuadTree(self, world):
        if world.simulator is not None:
            xmin = float("inf")
            xmax = float("-inf")
            ymin = float("inf")
            ymax = float("-inf")
            n = 0
    
            for location in world.locations:
                if world.simulator.id in location.positions:
                    n += 1
                    position = location.positions[world.simulator.id]
                    x = position[0]
                    y = position[1]
                    if x < xmin:
                        xmin = x
                    if x > xmax:
                        xmax = x
                    if y < ymin:
                        ymin = y
                    if y > ymax:
                        ymax = y

            if n == 0:
                n = 1
                        
            leafsize = 10
            depth = math.ceil(math.log(n / leafsize, 4))

            print('QuadTree for', n, 'positions, with depth', depth,
                  'and leaf size', leafsize)

            world.locationstree = cargoworld_quadtree.QuadTree(xmin, xmax,
                                                               ymin, ymax,
                                                               depth, leafsize)
            world.cargostree = cargoworld_quadtree.QuadTree(xmin, xmax,
                                                            ymin, ymax,
                                                            depth, leafsize)

            for location in world.locations:
                if world.simulator.id in location.positions:
                    position = location.positions[world.simulator.id]
                    x = position[0]
                    y = position[1]
                    world.locationstree.add(location, x, y)



    def dumpPlayerPosition(self, world, name, type):
        print("adding", name, " type ", type)
        filename = CARGOWORLD_DATABASE_BASENAME + "_positions_" + world.simulator.id + "_" + world.player.name + ".xml"
        files = glob.glob(filename)
        if not files:
            file = open(filename, "w")
            file.write("<?xml version=\"1.0\"?>\n<!DOCTYPE CARGOWORLD_DATABASE>\n<CARGOWORLD_DATABASE version=\"" + world.version + "\" author=\"" + world.player.name + "\">\n<POSITIONS simulator=\"" + world.simulator.id + "\">\n</POSITIONS>\n</CARGOWORLD_DATABASE>\n")
            file.close()
        
        tree = etree.parse(filename)
        root = tree.getroot()
        positions = tree.find('.//POSITIONS')       

        world.simulator.telemetry.lock.acquire()
        x = world.simulator.telemetry.x
        y = world.simulator.telemetry.y
        z = world.simulator.telemetry.z
        world.simulator.telemetry.lock.release()
        
        positionnode = tree.find(".//POSITION[@name='" + name + "']")
        if positionnode is None:
            positionnode = etree.SubElement(positions, 'POSITION')
            positionnode.set("name", name)

        positionnode.set("type", type)
        positionnode.set("x", str(x))
        positionnode.set("y", str(y))
        positionnode.set("z", str(z))

        tree.write(filename)
            
        location = self.addPosition(world, world.simulator, name, type, x, y, z)
        if location is not None:                
            world.locationstree.add(location, x, y)


    def dumpPlayerTrailer(self, world, type, cargoarea):
        print("adding", type, "cargoarea", cargoarea)
        filename = CARGOWORLD_DATABASE_BASENAME + "_trailers_" + world.player.name + ".xml"
        files = glob.glob(filename)
        if not files:
            file = open(filename, "w")
            file.write("<?xml version=\"1.0\"?>\n<!DOCTYPE CARGOWORLD_DATABASE>\n<CARGOWORLD_DATABASE version=\"" + world.version + "\" author=\"" + world.player.name + "\">\n<TRAILER_TYPES>\n</TRAILER_TYPES>\n</CARGOWORLD_DATABASE>\n")
            file.close()
        
        tree = etree.parse(filename)
        root = tree.getroot()
        trailers = tree.find('.//TRAILER_TYPES')       
        
        trailernode = tree.find(".//TRAILER_TYPE[@type='" + type + "']")
        if trailernode is None:
            trailernode = etree.SubElement(trailers, 'TRAILER_TYPE')
            trailernode.set("type", type)

        trailernode.set("cargoarea", cargoarea)

        tree.write(filename)
            
        world.trailertypes[type] = cargoarea
        

    def dumpDeliveredCargo(self, player):
        filename = "cargolist_" + player.name
        file = open(filename, "a+")
        for cargo in player.cargodelivered:
            file.write(cargo.name + "\t" + cargo.type.type + "\t" + cargo.location.name + "\t" + cargo.destination.name + "\n")
        file.close()
