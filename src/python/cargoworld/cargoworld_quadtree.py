
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

# used for testing in main
import random



# QUADTREE -------------------------------------------------------------------------


class QuadTree:
    'QuadTree'

   
    def __init__(self, xmin, xmax, ymin, ymax, depth=8, leafsize=1):
        self.depth = depth
        self.leafsize = leafsize
        
        self.xmin = xmin
        self.xmax = xmax
        self.ymin = ymin
        self.ymax = ymax
        
        self.nw = None
        self.ne = None
        self.sw = None
        self.se = None

        self.elements = {}


    def add(self, element, x, y):
        if (len(self.elements) < self.leafsize and self.nw is None) or self.depth == 0:
            self.elements[element] = [x, y]
        else:
            xmid = (self.xmin + self.xmax) * 0.5
            ymid = (self.ymin + self.ymax) * 0.5
            if self.nw is None:
                nd = self.depth - 1
                self.nw = QuadTree(self.xmin, xmid, ymid, self.ymax, nd, self.leafsize)
                self.ne = QuadTree(xmid, self.xmax, ymid, self.ymax, nd, self.leafsize)
                self.sw = QuadTree(self.xmin, xmid, self.ymin, ymid, nd, self.leafsize)
                self.se = QuadTree(self.xmin, xmid, ymid, self.ymax, nd, self.leafsize)
            if x < xmid:
                if y < ymid:
                    self.sw.add(element, x, y)
                else:
                    self.nw.add(element, x, y)
            else:
                if y < ymid:
                    self.se.add(element, x, y)
                else:
                    self.ne.add(element, x, y)
            for e in self.elements:
                ex = self.elements[e][0]
                ey = self.elements[e][1]
                if ex < xmid:
                    if ey < ymid:
                        self.sw.add(e, ex, ey)
                    else:
                        self.nw.add(e, ex, ey)
                else:
                    if ey < ymid:
                        self.se.add(e, ex, ey)
                    else:
                        self.ne.add(e, ex, ey)
            self.elements = {}


            
    def getNeighborhood(self, x, y):
        result = None
        if self.nw is None:
            result = self.elements
        else:
            if x < self.nw.xmax:
                if y < self.nw.ymin:
                    result = self.sw.getNeighborhood(x, y)
                else:
                    result = self.nw.getNeighborhood(x, y)
            else:
                if y < self.nw.ymin:
                    result = self.se.getNeighborhood(x, y)
                else:
                    result = self.ne.getNeighborhood(x, y)
        return list(result)


    def getAllElements(self):
        if self.nw is None:
            result = list(self.elements)
        else:
            result = self.sw.getAllElements() + self.nw.getAllElements() + self.se.getAllElements() + self.ne.getAllElements()

            
        return result
    
    
    def remove(self, element, x, y):
        if element in self.elements:
            del self.elements[element]
        else:
            if self.nw is not None:
                if x < self.nw.xmax:
                    if y < self.nw.ymin:
                        self.sw.remove(element, x, y)
                    else:
                        self.nw.remove(element, x, y)
                else:
                    if y < self.nw.ymin:
                        self.se.remove(element, x, y)
                    else:
                        self.ne.remove(element, x, y)

                        
    def removeRecursive(self, element):
        if element in self.elements:
            del self.elements[element]
        else:
            if self.nw is not None:
                self.sw.removeRecursive(element)
                self.nw.removeRecursive(element)
                self.se.removeRecursive(element)
                self.ne.removeRecursive(element)

            
    def display(self):
        print('Depth', self.depth, self.elements)
        if self.nw is not None:
            self.nw.display()
            self.ne.display()
            self.sw.display()
            self.se.display()
            

# MAIN ------------------------------------------------------------------------


if __name__ == '__main__':

    nbiter = 100
    
    xmax = 10
    xmin = -10
    ymax = 10
    ymin = -10

    depth = 8
    leafsize = 10
    
    q = QuadTree(xmin, xmax, ymin, ymax, depth, leafsize)
    
    for i in range(0,nbiter):
        x = random.random() * (xmax - xmin) + xmin
        y = random.random() * (ymax - ymin) + ymin
        q.add(i, x, y)
        
    q.display()

    nb = q.getNeighborhood(x, y)
    print(nb)

    q.remove(i, x, y)
    q.display()

    nb = q.getNeighborhood(x, y)
    print(nb)

    all = q.getAllElements()
    print(all)
