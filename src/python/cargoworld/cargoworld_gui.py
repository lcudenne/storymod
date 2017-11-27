
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






import sys, threading, time
from functools import partial
import webbrowser
import PyQt5
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *

import cargoworld

# DEFINITIONS ----------------------------------------------------------------------

TELEMETRY_TIMEOUT = 60

TELEMETRY_STYLE_RED = "background-color:rgb(255,180,180); color:rgb(255, 255, 255)"
TELEMETRY_STYLE_GREEN = "background-color:rgb(200,255,200); color:rgb(100, 100, 100)"
TELEMETRY_STYLE_GREY = "background-color:rgb(220,220,220); color:rgb(100, 100, 100)"
TELEMETRY_STYLE_LIGHTGREY = "background-color:rgb(240,240,240); color:rgb(100, 100, 100)"
TELEMETRY_STYLE_ORANGE = "background-color:rgb(255,200,100); color:rgb(255, 255, 255)"
TELEMETRY_STYLE_BLUE = "background-color:rgb(100,200,255); color:rgb(255, 255, 255)"
TELEMETRY_STYLE_LIGHTBLUE = "background-color:rgb(210,230,255); color:rgb(100, 100, 100)"

CARGOAREA_AREA_WIDTH = 750
CARGOAREA_CARGO_HEIGHT = 200

# ----------------------------------------------------------------------------------


class TrailerTypeWidget(QWidget):

    def __init__(self, world, parent = None):
        super(TrailerTypeWidget, self).__init__(parent)

        self.bgcolor = self.palette().color(QPalette.Background)
        
        self.setAutoFillBackground(True)
        p = self.palette()
        p.setColor(self.backgroundRole(), QColor(220,220,220))
        self.setPalette(p)

        self.world = world

        self.layout = QGridLayout()
        self.layout.setSpacing(10)
        self.setLayout(self.layout)
        
        self.headerWidget = QLabel()
        self.headerWidget.setText('Trailer')
        self.headerWidget.setAlignment(Qt.AlignLeft)
        font = QFont()
        font.setPointSize(14)
        font.setBold(True)
        self.headerWidget.setFont(font)

        self.trailerWidget = QLineEdit()
        self.trailerWidget.setText('')
        self.trailerWidget.setAlignment(Qt.AlignCenter)
        self.trailerWidget.setReadOnly(True)
        self.trailerWidget.setAutoFillBackground(True)
        self.trailerWidget.setStyleSheet(TELEMETRY_STYLE_LIGHTGREY)        

        self.trailerareaWidget = QLineEdit()
        self.trailerareaWidget.setText('')
        self.trailerareaWidget.setAlignment(Qt.AlignCenter)
        self.trailerareaWidget.setReadOnly(True)
        self.trailerareaWidget.setAutoFillBackground(True)
        self.trailerareaWidget.setStyleSheet(TELEMETRY_STYLE_LIGHTGREY)        

        self.dumpWidget = QLabel()
        self.dumpWidget.setText('Add current trailer')
        self.dumpWidget.setAlignment(Qt.AlignCenter)
        
        self.typeCombo = QComboBox()
        self.typeCombo.setInsertPolicy(QComboBox.InsertAlphabetically)
        for areatype in self.world.cargoareatypes:
            if areatype.lower().startswith(self.world.simulator.id.lower()):
                self.typeCombo.addItem(areatype)
        self.typeCombo.model().sort(0)
        
        self.dumpButton = QPushButton('Add trailer')
        self.dumpButton.setEnabled(True)
        self.dumpButton.clicked.connect(self.dumpTrailer)

        self.layout.addWidget(self.headerWidget, 1, 1)
        self.layout.addWidget(self.trailerWidget, 2, 1)
        self.layout.addWidget(self.trailerareaWidget, 3, 1)
        self.layout.addWidget(self.dumpWidget, 4, 1)
        self.layout.addWidget(self.typeCombo, 5, 1)
        self.layout.addWidget(self.dumpButton, 6, 1)        
        



    def dumpTrailer(self):
        self.world.interface.dumpPlayerTrailer(self.world, self.trailerWidget.text(),
                                               self.typeCombo.currentText())
        self.world.window.msgWidget.setText("trailer " + self.trailerWidget.text() + " (" + self.typeCombo.currentText() + ") added")


        
# ----------------------------------------------------------------------------------

class PositionWidget(QWidget):

    def __init__(self, world, parent = None):
        super(PositionWidget, self).__init__(parent)

        self.bgcolor = self.palette().color(QPalette.Background)
        
        self.setAutoFillBackground(True)
        p = self.palette()
        p.setColor(self.backgroundRole(), QColor(220,220,220))
        self.setPalette(p)

        self.world = world
        self.prevlocation = None

        self.layout = QGridLayout()
        self.layout.setSpacing(10)
        self.setLayout(self.layout)
        
        self.headerWidget = QLabel()
        self.headerWidget.setText('Position')
        self.headerWidget.setAlignment(Qt.AlignLeft)
        font = QFont()
        font.setPointSize(14)
        font.setBold(True)
        self.headerWidget.setFont(font)
        
        self.curposWidget = QLineEdit()
        self.curposWidget.setText('')
        self.curposWidget.setAlignment(Qt.AlignCenter)
        self.curposWidget.setReadOnly(True)
        self.curposWidget.setAutoFillBackground(True)
        self.curposWidget.setStyleSheet(TELEMETRY_STYLE_LIGHTGREY)        
        
        self.curtypeWidget = QLineEdit()
        self.curtypeWidget.setText('')
        self.curtypeWidget.setAlignment(Qt.AlignCenter)
        self.curtypeWidget.setReadOnly(True)
        self.curtypeWidget.setAutoFillBackground(True)
        self.curtypeWidget.setStyleSheet(TELEMETRY_STYLE_LIGHTGREY)        

        self.dumpWidget = QLabel()
        self.dumpWidget.setText('Add current position')
        self.dumpWidget.setAlignment(Qt.AlignCenter)

        self.posnameWidget = QLineEdit()
        self.posnameWidget.setText("country:city:name")

        self.typeCombo = QComboBox()
        self.typeCombo.setInsertPolicy(QComboBox.InsertAlphabetically)
        for loctype in self.world.locationtypes:
            self.typeCombo.addItem(loctype.type)
        self.typeCombo.model().sort(0)
        
        self.dumpButton = QPushButton('Add position')
        self.dumpButton.setEnabled(True)
        self.dumpButton.clicked.connect(self.dumpPosition)
        
        self.layout.addWidget(self.headerWidget, 1, 1)
        self.layout.addWidget(self.curposWidget, 2, 1)
        self.layout.addWidget(self.curtypeWidget, 3, 1)
        self.layout.addWidget(self.dumpWidget, 4, 1)
        self.layout.addWidget(self.posnameWidget, 5, 1)
        self.layout.addWidget(self.typeCombo, 6, 1)
        self.layout.addWidget(self.dumpButton, 7, 1)

        self.world.positionsignal.connect(self.updatePosition)

    def dumpPosition(self):
        self.world.dumpPosition(self.posnameWidget.text(), self.typeCombo.currentText())
        self.world.window.msgWidget.setText("position " + self.posnameWidget.text() + " (" + self.typeCombo.currentText() + ") added")
        
    def updatePosition(self):
        self.world.lock.acquire()
        location = self.world.player.closelocation

        if location is not None:
            self.curposWidget.setText(location.name)

            unload = False
            if self.world.player.cargoarea is not None:
                unload = self.world.player.cargoarea.isUnloadLocation(location)

            if unload:
                self.curposWidget.setStyleSheet(TELEMETRY_STYLE_GREEN)
                self.curtypeWidget.setStyleSheet(TELEMETRY_STYLE_GREEN)
            elif location.isShared():
                self.curposWidget.setStyleSheet(TELEMETRY_STYLE_LIGHTBLUE)
                self.curtypeWidget.setStyleSheet(TELEMETRY_STYLE_LIGHTBLUE)
            else:
                self.curposWidget.setStyleSheet(TELEMETRY_STYLE_LIGHTGREY)
                self.curtypeWidget.setStyleSheet(TELEMETRY_STYLE_LIGHTGREY)

            if location.type is not None:
                self.curtypeWidget.setText(location.type.type)
            else:
                self.curtypeWidget.setText('')
        else:
            self.curposWidget.setText('')
            self.curtypeWidget.setText('')
            self.curposWidget.setStyleSheet(TELEMETRY_STYLE_LIGHTGREY)
            self.curtypeWidget.setStyleSheet(TELEMETRY_STYLE_LIGHTGREY)

        self.world.lock.release()

        if location != self.prevlocation:
            self.world.window.refreshCargoList()
            self.world.window.refreshCargoarea(location)
            self.prevlocation = location




# ----------------------------------------------------------------------------------

class TelemetryWidget(QWidget):

    def __init__(self, simulator, parent = None):
        super(TelemetryWidget, self).__init__(parent)
        
        self.bgcolor = self.palette().color(QPalette.Background)

        self.firstupdate = True
        
        self.setAutoFillBackground(True)
        p = self.palette()
        p.setColor(self.backgroundRole(), QColor(220,220,220))
        self.setPalette(p)

        self.simulator = simulator
        self.world = self.simulator.world
        
        self.lastupdate = time.time() - TELEMETRY_TIMEOUT
        
        self.layout = QGridLayout()
        self.layout.setSpacing(10)
        self.setLayout(self.layout)

        self.headerWidget = QLabel()
        self.headerWidget.setText('Telemetry')
        self.headerWidget.setAlignment(Qt.AlignLeft)
        font = QFont()
        font.setPointSize(14)
        font.setBold(True)
        self.headerWidget.setFont(font)
        
        self.connectButton = QPushButton('Connect')
        self.connectButton.setEnabled(False)
        self.connectButtonSet = False
        
        self.telelogButton = QPushButton('Log telemetry to file')
        self.telelogButton.setEnabled(False)
        self.telelogButton.clicked.connect(self.switchTelemetryLog)

        self.clientWidget = QLineEdit()
        self.clientWidget.setText('not connected')
        self.clientWidget.setAlignment(Qt.AlignCenter)
        self.clientWidget.setReadOnly(True)
        self.clientWidget.setAutoFillBackground(True)
        self.clientWidget.setStyleSheet(TELEMETRY_STYLE_LIGHTGREY)        
        
        self.coordxWidget = QLabel()
        self.coordxWidget.setText('0')
        self.coordyWidget = QLabel()
        self.coordyWidget.setText('0')
        self.coordzWidget = QLabel()
        self.coordzWidget.setText('0')

        self.speedButton = QPushButton('0')
        self.speedButton.setEnabled(False)
        self.speedButton.setStyleSheet(TELEMETRY_STYLE_GREY)
        
        self.engineButton = QPushButton('Engine')
        self.engineButton.setEnabled(False)
        self.engineButton.setStyleSheet(TELEMETRY_STYLE_GREY)

        self.pbrakeButton = QPushButton('P.Brake')
        self.pbrakeButton.setEnabled(False)
        self.pbrakeButton.setStyleSheet(TELEMETRY_STYLE_GREY)

        self.lblinkButton = QPushButton('<')
        self.lblinkButton.setEnabled(False)
        self.lblinkButton.setStyleSheet(TELEMETRY_STYLE_GREY)

        self.trailerConnectedButton = QPushButton('trailer')
        self.trailerConnectedButton.setEnabled(False)
        self.trailerConnectedButton.setStyleSheet(TELEMETRY_STYLE_GREY)

        self.rblinkButton = QPushButton('>')
        self.rblinkButton.setEnabled(False)
        self.rblinkButton.setStyleSheet(TELEMETRY_STYLE_GREY)

        


        
        self.layout.addWidget(self.headerWidget, 1, 1, 1, 3)
        self.layout.addWidget(self.connectButton, 2, 1, 1, 3)
        self.layout.addWidget(self.clientWidget, 3, 1, 1, 3)
        self.layout.addWidget(self.telelogButton, 4, 1, 1, 3)
#        self.layout.addWidget(self.coordxWidget, 5, 1)
#        self.layout.addWidget(self.coordyWidget, 5, 2)
#        self.layout.addWidget(self.coordzWidget, 5, 3)
        self.layout.addWidget(self.engineButton, 5, 1)
        self.layout.addWidget(self.speedButton, 5, 2)
        self.layout.addWidget(self.pbrakeButton, 5, 3)
        self.layout.addWidget(self.lblinkButton, 6, 1)
        self.layout.addWidget(self.trailerConnectedButton, 6, 2)
        self.layout.addWidget(self.rblinkButton, 6, 3)

        self.simulator.telemetrysignal.connect(self.updateTelemetry)


        

    def switchTelemetryLog(self):
        self.simulator.telemetry.lock.acquire()
        self.simulator.telemetry.telelog = not self.simulator.telemetry.telelog
        switchtelelog = self.simulator.telemetry.telelog
        self.simulator.telemetry.lock.release()
        
        if switchtelelog:
            self.simulator.telemetry.openTelelog()
            self.telelogButton.setText('Logging telemetry to file')
            self.telelogButton.setStyleSheet(TELEMETRY_STYLE_RED)
        else:
            self.simulator.telemetry.closeTelelog()
            self.telelogButton.setText('Log telemetry to file')
            self.telelogButton.setStyleSheet("")
            
        
            
    def updateTelemetry(self):

        if self.firstupdate:
            self.connectButton.setText('Connected')
            self.connectButton.setEnabled(False)
            self.world.window.playbtn.setEnabled(True)
            self.firstupdate = False
        
        self.simulator.telemetry.lock.acquire()
        
        client = self.simulator.telemetry.clienturl + " client v" + str(self.simulator.telemetry.clientversionmaj) + "." + str(self.simulator.telemetry.clientversionmin)
        x = self.simulator.telemetry.x
        y = self.simulator.telemetry.y
        z = self.simulator.telemetry.z
        speed = self.simulator.telemetry.speed * self.simulator.speedmultiplier
        engine = self.simulator.telemetry.engineenabled == 1
        pbrake = self.simulator.telemetry.parkingbrake == 1
        lblink = self.simulator.telemetry.lblinker == 1
        rblink = self.simulator.telemetry.rblinker == 1
        trailerconnected = self.simulator.telemetry.trailerconnected == 1
        trailer = self.simulator.telemetry.trailer
        if self.simulator.telemetry.telelog is False:
            self.telelogButton.setText('Log telemetry to file')
            self.telelogButton.setEnabled(True)

        self.simulator.telemetry.lock.release()

        if self.clientWidget.text() != client:
            self.clientWidget.setText(client)
        
        self.coordxWidget.setText('%.2f' % x)
        self.coordyWidget.setText('%.2f' % y)
        self.coordzWidget.setText('%.2f' % z)
        self.speedButton.setText('%d' % int(speed))

        self.speedButton.setStyleSheet(TELEMETRY_STYLE_GREEN)
        if trailerconnected:
            if int(speed) > self.simulator.speedwarning:
                self.speedButton.setStyleSheet(TELEMETRY_STYLE_BLUE)
        if int(speed) > self.simulator.speedover:
            self.speedButton.setStyleSheet(TELEMETRY_STYLE_RED)

        if engine:
            self.engineButton.setStyleSheet(TELEMETRY_STYLE_GREEN)
        else:
            self.engineButton.setStyleSheet(TELEMETRY_STYLE_RED)

        if pbrake:
            self.pbrakeButton.setStyleSheet(TELEMETRY_STYLE_RED)
        else:
            self.pbrakeButton.setStyleSheet(TELEMETRY_STYLE_GREEN)

        if lblink:
            self.lblinkButton.setStyleSheet(TELEMETRY_STYLE_ORANGE)
        else:
            self.lblinkButton.setStyleSheet(TELEMETRY_STYLE_GREEN)

        if trailerconnected:
            self.trailerConnectedButton.setStyleSheet(TELEMETRY_STYLE_GREEN)
        else:
            self.trailerConnectedButton.setStyleSheet(TELEMETRY_STYLE_RED)

        if rblink:
            self.rblinkButton.setStyleSheet(TELEMETRY_STYLE_ORANGE)
        else:
            self.rblinkButton.setStyleSheet(TELEMETRY_STYLE_GREEN)
            

            
        if  self.world.window.trailertypeWidget.trailerWidget.text() != trailer:
            i = 0
            while i < self.world.window.cargoEntryListWidget.count():
                ca = self.world.window.cargoEntryListWidget.itemWidget(self.world.window.cargoEntryListWidget.item(i))
                self.world.window.cargoEntryListWidget.takeItem(i)
                i += 1
            self.world.window.trailertypeWidget.trailerWidget.setText(trailer)
            if self.world.player.cargoarea is not None:
                self.world.window.trailertypeWidget.trailerareaWidget.setText(self.world.player.cargoarea.type.type)
                self.world.window.trailertypeWidget.dumpButton.setEnabled(False)
            else:
                self.world.window.trailertypeWidget.dumpButton.setEnabled(True)

            
        self.lastupdate = time.time()



        
# ----------------------------------------------------------------------------------

class CargoEntryWidget(QWidget):

    def __init__(self, mainwindow, cargo, load, parent = None):
        super(CargoEntryWidget, self).__init__(parent)
        
        self.mainwindow = mainwindow
        self.cargo = cargo

        self.layout = QGridLayout()
        self.setLayout(self.layout)

        self.layout.setSpacing(0)
        self.setContentsMargins(0, 0, 0, 0)
        self.layout.setContentsMargins(0, 0, 0, 0)
        
        rect = QRect(0, 120, 200, 48)
        self.iconWidget = QLabel()
        px = QPixmap(cargo.type.picture)
        pxscale = px.scaled(200, 200, Qt.KeepAspectRatio)
        pxcrop = pxscale.copy(rect)
        self.iconWidget.setPixmap(pxcrop)

        self.nameWidget = QLabel()
        self.nameWidget.setText(cargo.name)

        if load:
            self.loadButton = QPushButton('Load')
            self.loadButton.setEnabled(False)
            self.loadButton.clicked.connect(partial(self.mainwindow.addCargoToArea, cargo))
        else:
            self.loadButton = QPushButton('Unload')
            self.loadButton.setEnabled(False)
            self.loadButton.clicked.connect(partial(self.mainwindow.removeCargoFromArea, cargo))

        self.loadButton.setFixedWidth(128)

        self.typeWidget = QLabel()
        self.typeWidget.setText(str(cargo.type.type))

        self.nbSlotsWidget = QLabel()
        self.nbSlotsWidget.setText("slots: " + str(cargo.type.nbslots))
        
        self.fromLocationWidget = QLabel()
        self.fromLocationWidget.setText(cargo.location.name)

        self.fromTypeWidget = QLabel()
        self.fromTypeWidget.setText(cargo.location.type.type)

        self.toLocationWidget = QLabel()
        self.toLocationWidget.setText(cargo.destination.name)

        self.toTypeWidget = QLabel()
        self.toTypeWidget.setText(cargo.destination.type.type)

        if not load:
            self.fromLocationWidget.setStyleSheet('''
            color: rgb(100, 100, 255);
            ''')
        else:
            self.toLocationWidget.setStyleSheet('''
            color: rgb(100, 100, 255);
            ''')

            

        self.layout.addWidget(self.iconWidget, 1, 1, 2, 1)
        self.layout.addWidget(self.nameWidget, 1, 2)
        self.layout.addWidget(self.loadButton, 2, 2)
        self.layout.addWidget(self.typeWidget, 1, 3)
        self.layout.addWidget(self.nbSlotsWidget, 2, 3)
        self.layout.addWidget(self.fromLocationWidget, 1, 4)
        self.layout.addWidget(self.toLocationWidget, 2, 4)
#        self.layout.addWidget(self.fromTypeWidget, 1, 5)        
#        self.layout.addWidget(self.toTypeWidget, 2, 5)        



# ----------------------------------------------------------------------------------

class AboutWindow(QWidget):
    
    def __init__(self, world):
        super().__init__()
        self.initUI()
        self.world = world
        self.layout = None
        self.bgcolor = self.palette().color(QPalette.Background)
        self.setWindowIcon(QIcon('img/cargoworld_icon.png'))
        self.layout = QGridLayout()
        self.layout.setSpacing(10)
        self.setLayout(self.layout)

        self.logoWidget = QLabel()
        px = QPixmap("img/cargoworld_logo.png")
        self.logoWidget.setPixmap(px)

        self.aboutText = QLabel()
        self.aboutText.setText('CargoWorld ' + cargoworld.CARGOWORLD_VERSION + ' (' + cargoworld.CARGOWORLD_VERSION_NAME + ')<br/><a href="https://sites.google.com/site/storymodsite">https://sites.google.com/site/storymodsite</a><br/><br/>SCS Software forum thread<br/><a href="https://forum.scssoft.com/viewtopic.php?f=34&t=246807">https://forum.scssoft.com/viewtopic.php?f=34&t=246807</a><br/><br/> This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with this program. If not, see <a href="http://www.gnu.org/licenses/">http://www.gnu.org/licenses/</a>.')
        self.aboutText.setFixedWidth(512)
        self.aboutText.setWordWrap(True)
        self.aboutText.setOpenExternalLinks(True)
               
        self.layout.addWidget(self.logoWidget, 1, 1, 1, 1)
        self.layout.addWidget(self.aboutText, 2, 1, 1, 1)
        

 
        
    def initUI(self):
        self.setGeometry(100, 100, 640, 640)
        windowname = 'CargoWorld ' + cargoworld.CARGOWORLD_VERSION + ' (' + cargoworld.CARGOWORLD_VERSION_NAME + ')'
        self.setWindowTitle(windowname)
        self.show()

    def closeEvent(self, event):
        event.accept()

# ----------------------------------------------------------------------------------

class CargoGraphWindow(QWidget):
    
    def __init__(self, world):
        super().__init__()
        self.initUI()
        self.world = world
        self.layout = None
        self.bgcolor = self.palette().color(QPalette.Background)
        self.setWindowIcon(QIcon('img/cargoworld_icon.png'))
        self.layout = QGridLayout()
        self.layout.setSpacing(10)
        self.setLayout(self.layout)

        self.instrText = QLabel()
        self.instrText.setText('Copy the code below into one of the following online Graphviz website to generate the CargoGraph:<br/><a href="https://dreampuf.github.io/GraphvizOnline/">https://dreampuf.github.io/GraphvizOnline/</a><br/><a href="http://www.webgraphviz.com/">http://www.webgraphviz.com/</a>')
        self.instrText.setFixedWidth(512)
        self.instrText.setWordWrap(True)
        self.instrText.setOpenExternalLinks(True)
        
        self.graphText = QLineEdit()
        self.graphText.setText(world.interface.cargograph.source())
        self.graphText.setFixedWidth(512)
        self.graphText.setReadOnly(True)
        
        self.layout.addWidget(self.instrText, 1, 1, 1, 1)
        self.layout.addWidget(self.graphText, 2, 1, 1, 1)
        

 
        
    def initUI(self):
        self.setGeometry(100, 100, 600, 128)
        self.setMaximumSize(600, 128)
        windowname = 'CargoWorld ' + cargoworld.CARGOWORLD_VERSION + ' (' + cargoworld.CARGOWORLD_VERSION_NAME + ')'
        self.setWindowTitle(windowname)
        self.show()
        
    def closeEvent(self, event):
        event.accept()


        
# ----------------------------------------------------------------------------------

class MainWindow(QWidget):
    
    def __init__(self, world):
        super().__init__()
        self.initUI()
        self.world = world
        self.worldthread = None
        self.layout = None
        self.cargoEntryListWidget = None
        self.cargoareaEntryListWidget = None
        self.telemetryWidget = None
        self.positionWidget = None
        self.bgcolor = self.palette().color(QPalette.Background)
        self.message = "ready"
        self.setWindowIcon(QIcon('img/cargoworld_icon.png'))

        self.setStyleSheet('QToolTip {color : black}')
        
        
    def initUI(self):      
        self.setGeometry(100, 100, 630, 320)
        self.setMaximumWidth(950)
        windowname = 'CargoWorld ' + cargoworld.CARGOWORLD_VERSION + ' (' + cargoworld.CARGOWORLD_VERSION_NAME + ')'
        self.setWindowTitle(windowname)
        self.show()

    def about(self):
        self.aboutW = AboutWindow(self.world)
        
        
    def closeEvent(self, event):
        self.terminateMainWindow()
        event.accept()
        
    def addSimulator(self, simulator, xmove):
        simbtn = QPushButton('', self)
        simulator.button = simbtn
        simbtn.setIcon(QIcon(simulator.logo))
        simbtn.setIconSize(QSize(280,280))
        simbtn.move(xmove, 10)
        simbtn.clicked.connect(partial(self.chooseSimulator, simulator))
        simbtn.show()

    def chooseSimulator(self, simulator):
        self.world.setSimulator(simulator)
        for sim in self.world.simulators:
            sim.button.setParent(None)
            
        px = QPixmap(self.world.simulator.logo)
        pxscale = px.scaled(280,280, Qt.KeepAspectRatio)
        simlogo = QLabel()
        simlogo.setPixmap(pxscale)
        self.layout = QVBoxLayout()
        self.layout.addWidget(simlogo)
        self.setLayout(self.layout)
        simlogo.move(10,10)
        simlogo.show()
        self.getSimulatorParameters()

    def getSimulatorParameters(self):
        nickvalue = "Nickname"
        hostvalue = "localhost"
        companylogo = self.world.simulator.logo
        showtrailer = True
        
        if self.world.interface.config is not None:
            config = self.world.interface.config
            if config.has_option(self.world.simulator.id, "nick"):
                nickvalue = config.get(self.world.simulator.id, "nick")
            if config.has_option(self.world.simulator.id, "host"):
                hostvalue = self.world.interface.config.get(self.world.simulator.id, "host")
            if config.has_option(self.world.simulator.id, "companylogo"):
                companylogo = self.world.interface.config.get(self.world.simulator.id, "companylogo")           
            if config.has_option(self.world.simulator.id, "showtrailer"):
                showtrailer = self.world.interface.config.get(self.world.simulator.id, "showtrailer") == "True"

        nicklabel = QLabel(self)
        nicklabel.move(280,10)
        nicklabel.setText("Nickname")
        nicklabel.show()
        nick = QLineEdit(self)
        nick.move(410,10)
        nick.setText(nickvalue)
        nick.show()
        hostlabel = QLabel(self)
        hostlabel.move(280,40)
        hostlabel.setText("Host IP")
        hostlabel.show()
        host = QLineEdit(self)
        host.move(410,40)
        host.setText(hostvalue)
        host.show()
        headerlabel = QLabel(self)
        headerlabel.move(280,70)
        headerlabel.setText("Show trailer")
        headerlabel.show()
        headercheck = QCheckBox('', self)
        headercheck.setChecked(showtrailer)
        headercheck.move(410, 70)
        headercheck.show()
        companylabel = QLabel(self)
        companylabel.move(280,100)
        companylabel.setText("Company logo")
        companylabel.show()
        company = QLineEdit(self)
        company.move(410,100)
        company.setText(companylogo)
        company.show()
        companybtn = QPushButton('Browse', self)
        companybtn.move(510,100)
        companybtn.clicked.connect(partial(self.getSimulatorParameterFile, company))
        companybtn.show()
        btn = QPushButton('Validate', self)
        btn.move(510,130)
        btn.clicked.connect(partial(self.setSimulatorParameters,
                                    nicklabel, nick, hostlabel, host,
                                    companylabel, company, companybtn,
                                    headerlabel, headercheck,
                                    btn))
        btn.show()

    def getSimulatorParameterFile(self, company):
        filename, _ = QFileDialog.getOpenFileName(self,
                                                  'Select company logo image file',
                                                  '.',
                                                  'Images (*.png *.jpg *.jpeg)')
        if filename != "":
            company.setText(filename)

        
    def setSimulatorParameters(self, nicklabel, nick, hostlabel, host,
                                    companylabel, company, companybtn,
                                    headerlabel, headercheck, btn):
        self.world.simulator.host = host.text()
        self.world.addPlayer(nick.text(), self.world.simulator)
        self.world.player.companylogo = company.text()
        self.world.player.showtrailer = headercheck.isChecked()
        nicklabel.setParent(None)
        nick.setParent(None)
        hostlabel.setParent(None)
        host.setParent(None)
        companylabel.setParent(None)
        company.setParent(None)
        companybtn.setParent(None)
        headerlabel.setParent(None)
        headercheck.setParent(None)
        btn.setParent(None)
        self.prepareMainWindow()


    def prepareMainWindow(self):
        QWidget().setLayout(self.layout)

        if self.world.player.showtrailer:
            self.setGeometry(self.frameGeometry().x(), self.frameGeometry().x(), 950, 900)

        self.layout = QGridLayout()
        self.setLayout(self.layout)

        self.layout.setSpacing(0)
 
        companylogoWidget = QLabel()
        px = QPixmap(self.world.player.companylogo)
        pxscale = px.scaled(280,280, Qt.KeepAspectRatio)
        companylogoWidget.setPixmap(pxscale)      
        companylogoWidget.setAlignment(Qt.AlignTop)
        
        self.aboutbtn = QPushButton('', self)
        self.aboutbtn.setIcon(QIcon("img/cargoworld_icon.png"))
        self.aboutbtn.setIconSize(QSize(280,280))
        self.aboutbtn.setStyleSheet('border:none')
        self.aboutbtn.clicked.connect(self.about)
        
        nickWidget = QLabel(self.world.player.name)
        nickWidget.setAlignment(Qt.AlignCenter)
        font = QFont()
        font.setPointSize(20)
        nickWidget.setFont(font)

        self.trailerWidget = QLabel()
        px = QPixmap("img/trailer_notrailer.png")
        self.trailerWidget.setPixmap(px)
        self.trailerWidget.setAlignment(Qt.AlignBottom)

        self.telemetryWidget = TelemetryWidget(self.world.simulator)
        self.positionWidget = PositionWidget(self.world)
        self.trailertypeWidget = TrailerTypeWidget(self.world)
        
        scrollWidgetCargo = QScrollArea()
        scrollWidgetCargo.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        scrollWidgetCargo.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        scrollWidgetCargo.setAlignment(Qt.AlignTop)
        scrollWidgetCargo.setWidgetResizable(True)

        scrolllayoutcargo = QGridLayout()
        scrolllayoutcargo.setSpacing(4)
        scrollWidgetCargo.setLayout(scrolllayoutcargo)

        self.cargoEntryListWidget = QListWidget()
        scrollWidgetCargo.layout().addWidget(self.cargoEntryListWidget)

        
        scrollWidgetCargoarea = QScrollArea()
        scrollWidgetCargoarea.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        scrollWidgetCargoarea.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        scrollWidgetCargoarea.setAlignment(Qt.AlignTop)
        scrollWidgetCargoarea.setWidgetResizable(True)

        scrolllayoutcargoarea = QGridLayout()
        scrolllayoutcargoarea.setSpacing(4)
        scrollWidgetCargoarea.setLayout(scrolllayoutcargoarea)

        self.cargoareaEntryListWidget = QListWidget()
        scrollWidgetCargoarea.layout().addWidget(self.cargoareaEntryListWidget)


        self.graphbtn = QPushButton('', self)
        self.graphbtn.setIcon(QIcon("img/cargo_default.png"))
        self.graphbtn.setIconSize(QSize(64,64))
        self.graphbtn.setStyleSheet('border:none')
        self.graphbtn.clicked.connect(self.graph)
        self.graphbtn.setToolTip('Generate and display the cargo graph.')
        self.playbtn = QPushButton('', self)
        self.playbtn.setIcon(QIcon("img/playbutton.png"))
        self.playbtn.setIconSize(QSize(64,64))
        self.playbtn.setStyleSheet('border:none')
        self.playbtn.clicked.connect(self.play)
        self.playbtn.setToolTip('Generate new cargo in the neighborhood.')
        
        self.pausebtn = QPushButton('', self)
        self.pausebtn.setIcon(QIcon("img/pausebutton.png"))
        self.pausebtn.setIconSize(QSize(64,64))
        self.pausebtn.setStyleSheet('border:none')
        self.pausebtn.clicked.connect(self.pause)

        self.quitbtn = QPushButton('Quit', self)
        self.quitbtn.clicked.connect(partial(self.terminateMainWindow))

        self.msgWidget = QLineEdit()
        self.msgWidget.setText(self.message)
        self.msgWidget.setAlignment(Qt.AlignLeft)
        self.msgWidget.setReadOnly(True)
        self.msgWidget.setAutoFillBackground(True)
        self.msgWidget.setStyleSheet(TELEMETRY_STYLE_LIGHTGREY)        

        self.slotindicators = [None]*self.world.cargoareamaxslots
        for i in range(0,self.world.cargoareamaxslots):
            indbtn = QPushButton('')
            indbtn.setEnabled(False)
            indbtn.setStyleSheet(TELEMETRY_STYLE_GREY)
            indbtn.setFixedWidth(int(CARGOAREA_AREA_WIDTH / self.world.cargoareamaxslots))
            self.slotindicators[i] = indbtn
        
        self.layout.setRowMinimumHeight(1, 20)

        for i in range(0,self.world.cargoareamaxslots):
            self.layout.setColumnMinimumWidth(2 + i, int(CARGOAREA_AREA_WIDTH / self.world.cargoareamaxslots))
        
        self.layout.setColumnMinimumWidth(6, 64)
        self.layout.setColumnMinimumWidth(7, 64)
        self.layout.setColumnMinimumWidth(8, 64)
        self.layout.setColumnMinimumWidth(9, 64)
        
        if self.world.player.showtrailer:
            self.layout.addWidget(companylogoWidget, 1, 1, 2, 1)
            self.layout.addWidget(self.trailerWidget, 1, 2, 2, self.world.cargoareamaxslots)
            self.layout.addWidget(self.aboutbtn, 1, 2 + self.world.cargoareamaxslots, 2, 4)

        for i in range(0,self.world.cargoareamaxslots):
            self.layout.addWidget(self.slotindicators[i], 1, 2 + i)
        
        self.layout.addWidget(nickWidget, 3, 1)
        self.layout.addWidget(self.telemetryWidget, 4, 1)
        self.layout.addWidget(self.positionWidget, 5, 1, 3, 1)
        self.layout.addWidget(self.trailertypeWidget, 4, 2 + self.world.cargoareamaxslots, 1, 4)
        
        self.layout.addWidget(scrollWidgetCargo, 3, 2, 2, self.world.cargoareamaxslots)
        self.layout.addWidget(scrollWidgetCargoarea, 5, 2, 3, self.world.cargoareamaxslots)        

        self.layout.addWidget(self.graphbtn, 6, 3 + self.world.cargoareamaxslots)
        self.layout.addWidget(self.playbtn, 6, 4 + self.world.cargoareamaxslots)
        self.layout.addWidget(self.pausebtn, 6, 5 + self.world.cargoareamaxslots)
        
        self.layout.addWidget(self.quitbtn, 7, 4 + self.world.cargoareamaxslots, 1, 2)

        self.layout.addWidget(self.msgWidget, 8, 1, 1, 5 + self.world.cargoareamaxslots)

        self.worldthread = threading.Thread(target = self.world.start)
        self.world.newcargosignal.connect(self.refreshCargoList)
        self.world.telemetrysignal.connect(self.refreshTelemetry)
        self.world.messagesignal.connect(self.updateMessage)
        self.world.cargoareasignal.connect(self.updateTrailerWidget)

        self.worldthread.start()
        self.pause()
        
        self.world.simulator.startTelemetry()



        
    def updateTrailerWidget(self):
        if self.world.player.cargoarea is None:
            self.trailertypeWidget.trailerareaWidget.setText('')
            px = QPixmap("img/trailer_notrailer.png")
            while self.cargoareaEntryListWidget.count() > 0:
                ca = self.cargoareaEntryListWidget.itemWidget(self.cargoareaEntryListWidget.item(0))
                self.cargoareaEntryListWidget.takeItem(0)
                ca.cargo.cargowidget.setParent(None)
                ca.cargo.cargowidget = None
            
        else:
            px = QPixmap(self.world.player.cargoarea.type.picture)
            self.trailertypeWidget.trailerWidget.setText(self.world.player.cargoarea.type.type)
            self.trailertypeWidget.trailerareaWidget.setText('')

        self.trailerWidget.setPixmap(px)
        self.trailerWidget.setAlignment(Qt.AlignBottom)

        for i in range(0,self.world.cargoareamaxslots):
            if self.world.player.cargoarea is not None and i < self.world.player.cargoarea.type.nbslots:
                self.slotindicators[i].setStyleSheet(TELEMETRY_STYLE_GREEN)
            else:
                self.slotindicators[i].setStyleSheet(TELEMETRY_STYLE_GREY)

        

    def updateMessage(self):
        if self.msgWidget is not None:
            self.msgWidget.setText(self.message)

    def graph(self):
        if self.world.interface.cargograph.graph is not None:
            if self.world.interface.which("dot") is not None:
                self.world.interface.cargograph.render()
                webbrowser.open(r'cargograph.pdf')
            else:                
                self.graphW = CargoGraphWindow(self.world)


    def play(self):
        self.world.lock.acquire()
        if self.world.state is 'PAUSED':
            self.world.state = 'RUNNING'
        self.world.lock.release()
        self.playbtn.setEnabled(False)
        self.pausebtn.setEnabled(True)

    def pause(self):
        self.world.lock.acquire()
        if self.world.state is 'RUNNING':
            self.world.state = 'PAUSED'
        self.world.lock.release()
        self.playbtn.setEnabled(True)
        self.pausebtn.setEnabled(False)

        
    def refreshCargoList(self):
        
        if self.cargoEntryListWidget is not None:
            pos = self.world.getPlayerPosition()
            if pos is not None:

                self.world.lock.acquire()
                cargolist = self.world.cargostree.getNeighborhood(pos[0], pos[1])
                self.world.lock.release()
                
                for cargo in cargolist:
                    i = 0
                    cafound = None
                    while i < self.cargoEntryListWidget.count() and cafound is None:
                        ca = self.cargoEntryListWidget.itemWidget(self.cargoEntryListWidget.item(i))
                        if ca.cargo.id == cargo.id:
                            cafound = ca
                        i += 1
                    if cafound is None:
                        if cargo.areaslot == -1:
                            if self.world.player.cargoarea is not None:
                                if self.world.player.cargoarea.type in cargo.type.cargoareatypes:
                                    self.addCargoList(cargo)
                            else:
                                self.addCargoList(cargo)
                    else:
                        if cargo.areaslot == -1:
                            if self.world.player.cargoarea is not None:
                                if not self.world.player.cargoarea.type in cargo.type.cargoareatypes:
                                    cafound.loadButton.setEnabled(False)
                                    cafound.loadButton.setText('Trailer type')
                                    caitem = self.cargoEntryListWidget.takeItem(i - 1)
                                    caitem = None
                                    
                        if cargo.areaslot > -1:
                            cafound.loadButton.setEnabled(False)
                            cafound.loadButton.setText('Loaded')
                            caitem = self.cargoEntryListWidget.takeItem(i - 1)
                            caitem = None
                            
                i = 0
                while i < self.cargoEntryListWidget.count():
                    ca = self.cargoEntryListWidget.itemWidget(self.cargoEntryListWidget.item(i))
                    j = 0
                    found = None
                    while j < len(cargolist) and found is None:
                        cargo = cargolist[j]
                        if ca.cargo.id == cargo.id:
                            found = ca.cargo
                        j += 1
                    if found is None:
                        caitem = self.cargoEntryListWidget.takeItem(i)
                        caitem = None
                    else:
                        if self.world.player.cargoarea is not None:
                            if self.world.player.cargoarea.type in ca.cargo.type.cargoareatypes:
                                self.world.lock.acquire()
                                if ca.cargo.location == self.world.player.closelocation:
                                    slot = self.world.player.cargoarea.getAvailableSlot(ca.cargo.type.nbslots)
                                    if slot > -1:
                                        ca.loadButton.setEnabled(True)
                                        ca.loadButton.setText('Load')
                                    else:
                                        ca.loadButton.setEnabled(False)
                                        ca.loadButton.setText('No space')
                                else:
                                    ca.loadButton.setEnabled(False)
                                    ca.loadButton.setText('Too far')
                                self.world.lock.release()
                                
                            else:
                                ca.loadButton.setEnabled(False)
                                ca.loadButton.setText('Trailer type')
                                    
                        
                    i += 1
                    
  
    def addCargoList(self, cargo):
        if self.cargoEntryListWidget is not None:
            cargoentry = CargoEntryWidget(self, cargo, True)
            qlistwidgetitem = QListWidgetItem(self.cargoEntryListWidget)
            qlistwidgetitem.setSizeHint(cargoentry.sizeHint())
            self.cargoEntryListWidget.addItem(qlistwidgetitem)
            self.cargoEntryListWidget.setItemWidget(qlistwidgetitem, cargoentry)

    def addCargoareaList(self, cargo):
        if self.cargoareaEntryListWidget is not None:
            cargoareaentry = CargoEntryWidget(self, cargo, False)

            self.world.lock.acquire()
            location = self.world.player.closelocation
            self.world.lock.release()
            if location.isShared():
                cargoareaentry.loadButton.setEnabled(True)

            qlistwidgetitem = QListWidgetItem(self.cargoareaEntryListWidget)
            qlistwidgetitem.setSizeHint(cargoareaentry.sizeHint())
            self.cargoareaEntryListWidget.addItem(qlistwidgetitem)
            self.cargoareaEntryListWidget.setItemWidget(qlistwidgetitem, cargoareaentry)

    def addCargoToArea(self, cargo):
        if self.world.player.cargoarea is not None:
            slot = self.world.player.cargoarea.getAvailableSlot(cargo.type.nbslots)
            if slot > -1:
                cargo.areaslot = slot
                self.world.player.cargoarea.addCargoToArea(cargo, slot)
                self.addCargoareaList(cargo)
                cargoWidget = QLabel()

                rect = QRect(0, 0, int(CARGOAREA_AREA_WIDTH / self.world.cargoareamaxslots)*cargo.type.nbslots, CARGOAREA_CARGO_HEIGHT)
                px = QPixmap(cargo.type.picture)
                pxscale = px.scaledToHeight(CARGOAREA_CARGO_HEIGHT)
                pxcrop = pxscale.copy(rect)
                cargoWidget.setPixmap(pxcrop)
                cargoWidget.setAlignment(Qt.AlignTop)
                cargo.cargowidget = cargoWidget
                if self.world.player.showtrailer:
                    self.layout.addWidget(cargoWidget, 2, 2 + slot, 1, cargo.type.nbslots)
                self.refreshCargoList()
                self.message = "cargo "+ cargo.name + " (" + cargo.type.type + ") loaded"
                self.updateMessage()
                for i in range(slot,slot + cargo.type.nbslots):
                    self.slotindicators[i].setStyleSheet(TELEMETRY_STYLE_RED)

            
                
    def removeCargoFromArea(self, cargo):
        for i in range(cargo.areaslot,cargo.areaslot + cargo.type.nbslots):
            self.slotindicators[i].setStyleSheet(TELEMETRY_STYLE_GREEN)
        
        self.world.player.cargoarea.removeCargoFromArea(cargo, self.world.player)
        cargo.areaslot = -1
        cargo.cargowidget.setParent(None)
        cargo.cargowidget = None
        
        i = 0
        found = False
        while not found and i < self.cargoareaEntryListWidget.count():
            ca = self.cargoareaEntryListWidget.itemWidget(self.cargoareaEntryListWidget.item(i))
            if ca.cargo.id == cargo.id:
                self.cargoareaEntryListWidget.takeItem(i)
                found = True

            i += 1
            
        self.message = "cargo "+ cargo.name + " (" + cargo.type.type + ") unloaded"
        self.world.removeCargoFromArea(cargo)
        self.refreshCargoList()

        self.updateMessage()

        
    def refreshCargoarea(self, location):
        i = 0
        while i < self.cargoareaEntryListWidget.count():
            ca = self.cargoareaEntryListWidget.itemWidget(self.cargoareaEntryListWidget.item(i))
            if ca.cargo.destination == location or (location is not None and location.isShared()):
                ca.loadButton.setEnabled(True)
            else:                
                ca.loadButton.setEnabled(False)
            i = i + 1

            

    def refreshTelemetry(self):
        if not self.telemetryWidget.connectButtonSet and self.world.simulator.telemetry is not None:
            self.telemetryWidget.connectButton.clicked.connect(partial(self.world.simulator.telemetry.connectToSimulator))
            self.telemetryWidget.connectButtonSet = True

        if time.time() - self.telemetryWidget.lastupdate > TELEMETRY_TIMEOUT:
            self.telemetryWidget.firstupdate = True
            self.telemetryWidget.connectButton.setText('Connect')
            self.telemetryWidget.connectButton.setEnabled(True)
            if self.telemetryWidget.clientWidget.text() != 'not connected':
                self.telemetryWidget.clientWidget.setText('not connected')
            
            self.world.simulator.telemetry.closeTelelog()
            self.telemetryWidget.telelogButton.setEnabled(False)
            self.telemetryWidget.telelogButton.setText('Log telemetry to file')
            self.telemetryWidget.telelogButton.setStyleSheet("")
            self.telemetryWidget.coordxWidget.setText('0')
            self.telemetryWidget.coordyWidget.setText('0')
            self.telemetryWidget.coordzWidget.setText('0')
            self.telemetryWidget.speedButton.setText('0')
            self.telemetryWidget.speedButton.setStyleSheet(TELEMETRY_STYLE_GREY)
            self.telemetryWidget.engineButton.setStyleSheet(TELEMETRY_STYLE_GREY)
            self.telemetryWidget.pbrakeButton.setStyleSheet(TELEMETRY_STYLE_GREY)
            self.telemetryWidget.lblinkButton.setStyleSheet(TELEMETRY_STYLE_GREY)
            self.telemetryWidget.trailerConnectedButton.setStyleSheet(TELEMETRY_STYLE_GREY)
            self.telemetryWidget.rblinkButton.setStyleSheet(TELEMETRY_STYLE_GREY)            
            self.trailertypeWidget.trailerWidget.setText('')
            self.trailertypeWidget.trailerareaWidget.setText('')            
            self.trailertypeWidget.dumpButton.setEnabled(False)
            self.positionWidget.dumpButton.setEnabled(False)
            self.pause()
            self.playbtn.setEnabled(False)
        else:
            self.positionWidget.dumpButton.setEnabled(True)
        
        
    def terminateMainWindow(self):
        self.world.interface.configWrite(self.world)
        if self.telemetryWidget is not None:
            self.telemetryWidget.connectButton.setEnabled(False)
            self.telemetryWidget.telelogButton.setEnabled(False)
            self.trailertypeWidget.dumpButton.setEnabled(False)
            self.positionWidget.dumpButton.setEnabled(False)
            self.playbtn.setEnabled(False)
            self.pausebtn.setEnabled(False)
            self.quitbtn.setEnabled(False)
        self.world.terminate()
        if self.worldthread is not None:
            self.worldthread.join()
        if self.world.simulator is not None and self.world.simulator.telemetrythread is not None:
            self.world.simulator.telemetrythread.join()
        sys.exit()


# ----------------------------------------------------------------------------------
