//                            Package   : omniEvents
// PersistNode.cc             Created   : 2004/04/29
//                            Author    : Alex Tingle
//
//    Copyright (C) 2004 Alex Tingle.
//
//    This file is part of the omniEvents application.
//
//    omniEvents is free software; you can redistribute it and/or
//    modify it under the terms of the GNU Lesser General Public
//    License as published by the Free Software Foundation; either
//    version 2.1 of the License, or (at your option) any later version.
//
//    omniEvents is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//    Lesser General Public License for more details.
//
//    You should have received a copy of the GNU Lesser General Public
//    License along with this library; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "PersistNode.h"

#include <stdlib.h>
#include <stdio.h>
#include <climits>

namespace OmniEvents {


PersistNode::PersistNode(istream& is)
{
  while( readnode(is) ){}
}

PersistNode::~PersistNode()
{
  for(map<string,PersistNode*>::iterator i=_child.begin(); i!=_child.end(); ++i)
      delete i->second;
}

void PersistNode::output(ostream& os,string name) const
{
  if(!name.empty()) // Don't output root node.
  {
    os<<name<<'\n';
    for(map<string,string>::const_iterator i=_attr.begin();
        i!=_attr.end();
        ++i)
    {
      os<<" "<<i->first<<"="<<i->second<<'\n';
    }
    os<<" ;;\n";
    name+="/";
  }
  for(map<string,PersistNode*>::const_iterator i=_child.begin();
      i!=_child.end();
     ++i)
  {
    i->second->output(os,name+i->first);
  }
}


inline bool PersistNode::readnode(istream& is)
{
  PersistNode* node =NULL;
  string tok;
  while(true)
  {
    if(!readtoken(is,tok) || tok==";;")
        return bool(node);
    else if(node)
        node->addattr(tok);
    else if(tok[0]=='-')
        delnode(tok.substr(1));
    else
        node=addnode(tok);
  }
}

inline bool PersistNode::readtoken(istream& is, string& tok)
{
  while(is)
  {
    is>>tok;
    if(tok.empty())
        break;
    if(tok[0]!='#')
        return true;
    is.ignore(INT_MAX,'\n');
  }
  return false;
}

PersistNode* PersistNode::addnode(const string& name)
{
  string::size_type pos =name.find('/');
  // get reference to Next node in the path.
  PersistNode*& newchild =_child[name.substr(0,pos)];

  if(pos==string::npos) // leaf: add new leaf.
  {
    if(newchild)
        delete newchild; // overwrite old leaf (and its children)
    newchild=new PersistNode();
    return newchild;
  }
  else // branch: just add the branch if it's missing, and then recurse.
  {
    if(!newchild)
        newchild=new PersistNode();
    return newchild->addnode(name.substr(pos+1));
  }
}

void PersistNode::delnode(const string& name)
{
  string::size_type pos =name.find('/');
  // get reference to Next node in the path.
  map<string,PersistNode*>::iterator childpos =_child.find(name.substr(0,pos));
  if(childpos!=_child.end())
  {
    if(pos==string::npos) // leaf: delete leaf.
    {
      delete childpos->second;
      _child.erase(childpos);
    }
    else // branch: recurse
    {
      childpos->second->delnode(name.substr(pos+1));
    }
  }
}

void PersistNode::addattr(const string& keyvalue)
{
  string::size_type pos =keyvalue.find('=');
  _attr[keyvalue.substr(0,pos)]=(pos==string::npos?"":keyvalue.substr(pos+1));
}

void PersistNode::addattr(const string& key, long value)
{
  char buf[64];
  sprintf(buf,"%i",value);
  _attr[key]=string(buf);
}

bool PersistNode::hasAttr(const string& key) const
{
  return( _attr.find(key)!=_attr.end() );
}
string PersistNode::attrString(const string& key, const string& fallback) const
{
  map<string,string>::const_iterator pos=_attr.find(key);
  if(pos==_attr.end())
      return fallback;
  else
      return pos->second;
}
long PersistNode::attrLong(const string& key, long fallback) const
{
  map<string,string>::const_iterator pos=_attr.find(key);
  if(pos==_attr.end())
      return fallback;
  else
      return atol(pos->second.c_str());
}
PersistNode* PersistNode::child(const string& key) const
{
  map<string,PersistNode*>::const_iterator pos=_child.find(key);
  if(pos==_child.end())
      return NULL;
  else
      return pos->second;
}

}; // end namespace OmniEvents
