#                            Package   : omniEvents
# docbook2text.py            Created   : 2004/08/13
#                            Author    : Alex Tingle
#
#    Copyright (C) 2004,2005 Alex Tingle.
#
#    This file is part of the omniEvents application.
#
#    omniEvents is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Lesser General Public
#    License as published by the Free Software Foundation; either
#    version 2.1 of the License, or (at your option) any later version.
#
#    omniEvents is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with this library; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

import sys
import string
import getopt
import xml.sax
import xml.sax.handler


def _db(message):
  pass


class Element:
  def __init__(self,handler,name,attrs):
      self.handler=handler
      self.name=name
      self.attrs=attrs
      self.margin_left=0
      if handler.stack:
        self.parent=handler.stack[-1]
      else:
        self.parent=None

  def characters(self,content):
      """Pass content to parent content, by default."""
      if self.parent:
        self.parent.characters(content)

  def end(self):
      pass
      
  def ancestor(self,names):
      e=self.parent
      while e:
        if e.name in names:
          return e
        e=e.parent
      return None


class Section(Element):
  def __init__(self,handler,name,attrs):
      Element.__init__(self,handler,name,attrs)
      self.depth=0
      e=self.ancestor([name])
      if e:
        self.depth=e.depth+1
      if self.depth<len(handler.section):
        handler.section[self.depth]+=1
      else:
        handler.section.append(1)
      
  def end(self):
      for d in range(self.depth+1,len(handler.section)):
        handler.section[d]=0


class VariableList(Element):
  def __init__(self,handler,name,attrs):
      Element.__init__(self,handler,name,attrs)
      handler.margin_left+=4
      
  def end(self):
      self.handler.margin_left-=4


class LiteralLayout(Element):
  def __init__(self,handler,name,attrs):
      Element.__init__(self,handler,name,attrs)
      self.marginStr=' '*handler.margin_left
      self.handler.outfile.write(self.marginStr)

  def characters(self,content):
      content=string.replace(content,'\n','\n'+self.marginStr)
      self.handler.outfile.write(content)

  def end(self):
      self.handler.outfile.write('\n\n')
      self.handler.cursor=0


class Para(Element):
  def __init__(self,handler,name,attrs):
      Element.__init__(self,handler,name,attrs)
      self.space=1
      
  def tomargin(self):
      h=self.handler
      if h.cursor>h.margin_left:
        h.outfile.write('\n'+' '*h.margin_left)
      else:
        h.outfile.write(' '*(h.margin_left-handler.cursor))
      h.cursor=h.margin_left
      self.space=1
      
  def addspace(self):
      """Add a space."""
      if not self.space:
        h=self.handler
        if h.cursor<h.margin_left or h.cursor>=h.columns:
          self.tomargin()
        else:
          h.outfile.write(' ')
          h.cursor+=1
        self.space=1

  def addword(self,word):
      h=self.handler
      if h.cursor<h.margin_left or \
         len(word)+h.cursor>h.columns:
        self.tomargin()
      h.outfile.write(word)
      h.cursor+=len(word)
      self.space=0

  def characters(self,content):
      h=self.handler
      if content[0] in string.whitespace:
        self.addspace()
      first=1
      for word in string.split(content):
        if not first:
          self.addspace()
        first=0
        self.addword(word)
      if content[-1] in string.whitespace:
        self.addspace()

  def end(self):
      self.handler.outfile.write('\n\n')
      self.handler.cursor=0


class Title(Para):
  def __init__(self,handler,name,attrs):
      Para.__init__(self,handler,name,attrs)
      self.width=0;
      self.titleof=self.ancestor(['section','example','refsect1'])
      if self.titleof and self.titleof.name=='section':
        if self.titleof.depth>0:
          x=[]
          for i in range(0,self.titleof.depth+1):
            x.append(str(handler.section[i]))
          Para.characters(self,string.join(x,'.')+' ')
        else:
          Para.characters(self,str(handler.section[0])+'. ')
      elif self.titleof and self.titleof.name=='example':
          Para.characters(self,'Example: ')
      elif self.titleof and self.titleof.name=='refsect1':
        handler.margin_left-=3

  def characters(self,content):
      if self.titleof and self.titleof.name=='refsect1':
        content=string.upper(content)
      Para.characters(self,content)
      self.width=max(self.width,self.handler.cursor)
  
  def end(self):
      t=self.titleof
      if t and t.name=='section' and t.depth==0:
        self.tomargin()
        Para.characters(self,'='*self.width)
      elif t and t.name=='example':
        pass
      elif t and t.name=='refsect1':
        handler.margin_left+=3
      else:
        self.tomargin()
        Para.characters(self,'-'*self.width)
      Para.end(self)


class Span(Element):
  def __init__(self,handler,name,attrs):
      Element.__init__(self,handler,name,attrs)
      self.content=''

  def characters(self,content):
      self.content+=content
  
  def end(self):
      self.parent.characters(self.content)


class Quote(Span):
  def __init__(self,handler,name,attrs):
      Span.__init__(self,handler,name,attrs)

  def end(self):
      self.parent.characters('"'+self.content+'"')


class Command(Span):
  def __init__(self,handler,name,attrs):
      Span.__init__(self,handler,name,attrs)

  def end(self):
      self.parent.characters('`'+self.content+"'")


class Replaceable(Span):
  def __init__(self,handler,name,attrs):
      Span.__init__(self,handler,name,attrs)

  def end(self):
      self.parent.characters('<'+self.content+'>')


class OrderedList(Element):
  def __init__(self,handler,name,attrs):
      Element.__init__(self,handler,name,attrs)
      self.index=0


class ListItem(Para):
  def __init__(self,handler,name,attrs):
      Para.__init__(self,handler,name,attrs)
      self.tomargin()
      self.indent=3
      if self.parent and self.parent.name=='itemizedlist':
        handler.outfile.write(' o ')
      elif self.parent and self.parent.name=='orderedlist':
        self.parent.index+=1
        index=' %s) '%str(self.parent.index)
        handler.outfile.write(index)
        self.indent=len(index)
      else:
        handler.outfile.write('   ')
      handler.margin_left+=self.indent
      handler.cursor=handler.margin_left

  def end(self):
      handler.margin_left-=self.indent


class Term(Para):
  def __init__(self,handler,name,attrs):
      Para.__init__(self,handler,name,attrs)
      handler.margin_left-=3
      self.tomargin()
      #handler.outfile.write(' * ')
      #handler.margin_left+=3
      #handler.cursor=handler.margin_left

  def end(self):
      self.characters(':')
      self.handler.margin_left+=3
  

class InformalTable(Para):
  def __init__(self,handler,name,attrs):
      Para.__init__(self,handler,name,attrs)
      self.content=[]
      self.columns=0

  def characters(self,content):
      pass

  def end(self):
      colwidth=[0]*self.columns
      for row in self.content:
        for i in range(0,len(row)):
          colwidth[i]=max(colwidth[i],len(row[i]))
      # Emit table.
      for row in self.content:
        self.tomargin()
        for i in range(0,len(row)):
          self.handler.outfile.write('  '+string.ljust(row[i],colwidth[i]))
        self.handler.outfile.write('\n')
        self.handler.cursor=0
      self.handler.outfile.write('\n')
      self.handler.cursor=0


class Row(Element):
  def __init__(self,handler,name,attrs):
      Element.__init__(self,handler,name,attrs)
      self.informaltable=self.ancestor(['informaltable'])
      self.informaltable.content.append([])

  def end(self):
      if self.parent.name=='thead':
        divider=[]
        for e in self.informaltable.content[-1]:
          divider.append('-'*len(e))
        self.informaltable.content.append(divider)
        

class Entry(Element):
  def __init__(self,handler,name,attrs):
      Element.__init__(self,handler,name,attrs)
      self.informaltable=self.ancestor(['informaltable'])
      it=self.informaltable
      it.content[-1].append('')
      it.columns=max(it.columns,len(it.content[-1]))

  def characters(self,content):
      cleancontent=string.join(string.split(content))
      if not cleancontent:
        return
      row=self.informaltable.content[-1]
      if cleancontent[0] not in ' )]' and \
         row[-1] and row[-1][-1] not in ' ([':
        row[-1]+=' '
      row[-1]+=cleancontent


class RefNameDiv(Para):
  def __init__(self,handler,name,attrs):
      Para.__init__(self,handler,name,attrs)
      Para.characters(self,'-'*handler.columns)


class RefName(Span):
  def __init__(self,handler,name,attrs):
      Span.__init__(self,handler,name,attrs)

  def end(self):
      self.parent.characters(self.content+' - ')


class RefSect1(Element):
  def __init__(self,handler,name,attrs):
      Element.__init__(self,handler,name,attrs)
      handler.margin_left+=3

  def end(self):
      handler.margin_left-=3


elementMap={
  'title'         :Title,
  'section'       :Section,
  'literallayout' :LiteralLayout,
  'programlisting':LiteralLayout,
  'para'          :Para,
  'quote'         :Quote,
  'command'       :Command,
  'literal'       :Command,
  'orderedlist'   :OrderedList,
  'listitem'      :ListItem,
  'term'          :Term,
  'informaltable' :InformalTable,
  'row'           :Row,
  'entry'         :Entry,
  'refnamediv'    :RefNameDiv,
  'refname'       :RefName,
  'refsect1'      :RefSect1,
  'cmdsynopsis'   :Para,
  'replaceable'   :Replaceable,
  'variablelist'  :VariableList
}


class Docbook2TextHandler(xml.sax.handler.ContentHandler): 
  def __init__(self):
      self.outfile=sys.stdout
      self.columns=80
      self.stack=[]
      self.margin_left=0
      self.cursor=0
      self.section=[0]

  def set_columns(self,c):
      self.columns=c

  def set_outfile(self,filename):
      self.outfile=open(filename)

  def startDocument(self):
      _db('startDocument')

  def endDocument(self):
      _db('endDocument')

  def startElement(self,name,attrs):
      _db('-'*len(self.stack)+name)
      element=None
      if elementMap.has_key(name):
        element=elementMap[name](self,name,attrs)
      else:
        element=Element(self,name,attrs)
      self.stack.append(element)
      self.margin_left+=self.stack[-1].margin_left

  def endElement(self,name):
      assert(self.stack)
      assert(name==self.stack[-1].name)
      self.stack[-1].end()
      self.margin_left-=self.stack[-1].margin_left
      del self.stack[-1]

  def characters(self,content):
      if isinstance(content,unicode):
        content=content.encode("iso-8859-1", "replace")
      self.stack[-1].characters(content)
#end class Docbook2TextHandler


def usage():
  sys.stderr.write("""
Convert a DocBook XML file into text.

syntax: python docbook2text.py OPTIONS FILES

OPTIONS:                                         DEFAULT:
 -c COLUMNS  Width of the output.                 80
 -o FILE     Filename for the output.            standard output
 -v          Be verbose.
 -h          Show this text.
""")

################################################################################
# If this file is executed directly, then we start here.
if(__name__=="__main__"):
  handler=Docbook2TextHandler()

  try:
    opts,args=getopt.getopt(sys.argv[1:],"c:o:vh")
  except getopt.error:
    # print help information and exit:
    usage()
    sys.exit(-1)
  for option, optarg in opts:
    if option=='-c':
      handler.set_columns(optarg)
    elif option=='-o':
      handler.set_outfile(optarg)
    elif option=='-v':
      _db=lambda message: sys.stderr.write(message+'\n')
    elif option=='-h':
      usage()
      sys.exit(0)
    else:
      usage()
      sys.exit(-1)

  if args:
    for f in args:
      xml.sax.parse(f,handler)
  else:
      xml.sax.parse(sys.stdin,handler)
