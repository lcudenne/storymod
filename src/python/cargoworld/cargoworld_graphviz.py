
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



from graphviz import Digraph

# ----------------------------------------------------------------------------------



class CargoGraph():

    def __init__(self):

        self.graph = None
        self.colors = {
            'france': '#004fce'}

        

    def create(self):
        if self.graph is None:
            self.graph = Digraph(strict = 'true')
            self.graph.attr(label='The Cargo Graph', concentrate = 'true')
            self.graph.node_attr.update(shape='box', style='filled', color='grey')
            self.graph.edge_attr.update(penwidth='5')
            
        
    def addLocation(self, location):
        self.create()
        self.graph.node(location.name)

    def addCargo(self, cargo, playerlocation = None):
        self.create()
        ccolor = 'grey'
        loclist = cargo.location.name.split(':')
        if len(loclist) > 1 and loclist[0] in self.colors:
            ccolor = self.colors[loclist[0]]
        self.graph.edge(cargo.location.name.replace(":","_"), cargo.destination.name.replace(":","_"), label=cargo.type.type, color=ccolor)

        if cargo.location == playerlocation:
            self.graph.node(cargo.location.name.replace(":","_"), color=ccolor, fontcolor='white')

            

    def snapshot(self, world):        
        self.graph = None
        cargolist = world.cargostree.getAllElements()
        for cargo in cargolist:
            self.addCargo(cargo)        

    def render(self):
        self.create()
        self.graph.format = 'pdf'
        self.graph.render('cargograph')


    def world(self, world):
        graph = Digraph(strict = 'true')
        graph.attr(label='The CargoWorld Graph', concentrate = 'true')
        graph.node_attr.update(shape='box', style='filled', color='grey')
        graph.edge_attr.update(penwidth='5')

        for loctype in world.locationtypes:
            if loctype.outputcargo is not None:
                for i in range(0, len(loctype.outputcargo)):
                    cargotype = loctype.outputcargo[i]
                    for destloctype in world.locationtypes:
                        if destloctype != loctype:
                            if destloctype.isCargoInput(cargotype):
                                graph.edge(loctype.type, destloctype.type, label=cargotype)

        graph.format = 'pdf'
        graph.render('typesgraph')
