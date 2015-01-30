#ifndef OMNIEVENTS_ITERATOR_H
#define OMNIEVENTS_ITERATOR_H

#include <omniORB4/CORBA.h>
#include <ossie/CorbaUtils.h>
#include "CorbaGC.h"

namespace omniEvents {

        template <class Item, class ItemOut, class List, class ListOut, class IteratorType, class IteratorPOA, int ttl=60>
        class Iterator : public virtual IteratorPOA
        {
        public:
            Iterator(List* list) :
                list_(list),
                offset_(0)
            {
            }

            virtual ~Iterator()
            {
                delete list_;
            }

            virtual CORBA::Boolean next_one(ItemOut item)
            {
                if (offset_ >= list_->length()) {
                    item = new Item();
                    return false;
                }

                item = new Item((*list_)[offset_++]);
                return true;
            }

            virtual CORBA::Boolean next_n(CORBA::ULong how_many, ListOut bl)
            {
                if (offset_ >= list_->length()) {
                    bl = new List();
                    return false;
                }

                List& list = *list_;
                const CORBA::ULong length = list.length();
                how_many = std::min(how_many, length-offset_);
                bl = new List(how_many, how_many, &(list[offset_]), false);

                offset_ += how_many;
                return true;
            }

            virtual void destroy()
            {
            }

            static typename IteratorType::_ptr_type list(CORBA::ULong count, ListOut list, List* items)
            {
                if (count >= items->length()) {
                    list = items;
                    return IteratorType::_nil();
                }
                Iterator* iter = new Iterator(items);

                // Let the iterator handle fetching the list
                iter->next_n(count, list);

                // Activate the iterator into the garbage-collected POA
                PortableServer::POA_var poa = ossie::corba::RootPOA()->find_POA("Iterators", 1);
                CORBA::Object_var obj = ossie::corba::activateGCObject(poa, iter, boost::posix_time::seconds(ttl));
                iter->_remove_ref();

                return IteratorType::_narrow(obj);
            }

        private:
            List* list_;
            CORBA::ULong offset_;
        };
}


#endif
