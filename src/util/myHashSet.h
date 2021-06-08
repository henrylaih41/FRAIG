/****************************************************************************
  FileName     [ myHashSet.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashSet ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_SET_H
#define MY_HASH_SET_H

#include <vector>

using namespace std;

//---------------------
// Define HashSet class
//---------------------
// To use HashSet ADT,
// the class "Data" should at least overload the "()" and "==" operators.
//
// "operator ()" is to generate the hash key (size_t)
// that will be % by _numBuckets to get the bucket number.
// ==> See "bucketNum()"
//
// "operator ==" is to check whether there has already been
// an equivalent "Data" object in the HashSet.
// Note that HashSet does not allow equivalent nodes to be inserted
//
template <class Data>
class HashSet
{
public:
   HashSet(size_t b = 0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashSet() { reset(); }

   // TODO: implement the HashSet<Data>::iterator
   // o An iterator should be able to go through all the valid Data
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashSet<Data>;

   public:
       iterator(Data d = 0,size_t p = 0,size_t numB = 0, vector<Data>* bucketpointer = 0): pos(p),_data(d), numBuckets(numB), buckets(bucketpointer){}
       const Data& operator * () const { return _data; }
       
       iterator& operator ++ () {
           /*for(int i = 0;i < buckets[IbucketNum(_data)].size(); i++){
               if(buckets[IbucketNum(_data)][i] == _data){
                   if(i != buckets[IbucketNum(_data)].size() - 1){
                       Data buf(buckets[IbucketNum(_data)][i+1].getName(),buckets[IbucketNum(_data)][i+1].getLoad());
                       _data = buf;
                       return (*this);
                   }
                   else{ //look for next non-empty buckets's first element
                       int ID = IbucketNum(_data) + 1;
                       if (ID == numBuckets){
                           end = true; return (*this);
                       }
                       while(buckets[ID].size() == 0){
                           ID++;
                           if (ID == numBuckets){
                               end = true; return (*this);
                           }
                       }
                       Data buf(buckets[ID][0].getName(),buckets[ID][0].getLoad());
                       _data = buf;
                       return (*this);
                   }
               }
           }*/
        
                   if(pos != buckets[IbucketNum(_data)].size() - 1){
                       Data buf(buckets[IbucketNum(_data)][pos+1].getName(),buckets[IbucketNum(_data)][pos+1].getLoad());
                       _data = buf;
                       pos++;
                       return (*this);
                   }
                   else{ //look for next non-empty buckets's first element
                       int ID = IbucketNum(_data) + 1;
                       if (ID == numBuckets){
                           end = true; return (*this);
                       }
                       while(buckets[ID].size() == 0){
                           ID++;
                           if (ID == numBuckets){
                               end = true; return (*this);
                           }
                       }
                       Data buf(buckets[ID][0].getName(),buckets[ID][0].getLoad());
                       _data = buf;
                       pos = 0;
                       return (*this);
                   }
           
       }
       
       size_t IbucketNum(Data d){
           return d() % numBuckets;
       }
       
       iterator operator ++ (int) {
           iterator buf = *this;
           for(int i = 0;i < buckets[IbucketNum(_data)].size(); i++){
               if(buckets[IbucketNum(_data)][i] == _data){
                   if(i != buckets[IbucketNum(_data)].size() - 1){
                       Data buf(buckets[IbucketNum(_data)][i+1].getName(),buckets[IbucketNum(_data)][i+1].getLoad());
                       _data = buf;
                       return buf;
                   }
                   else{ //look for next non-empty buckets's first element
                       int ID = IbucketNum(_data) + 1;
                       while(buckets[ID].size() == 0){
                           ID++;
                           if (ID == numBuckets){
                               end = true; return buf;
                           }
                       }
                       Data buf(buckets[ID][0].getName(),buckets[ID][0].getLoad());
                       _data = buf;
                   }
               }
           }
           return buf;
       }
       
       /*iterator& operator = (const iterator& i) {
           numBuckets = i.numBuckets;
           buckets = i.buckets;
           _data = i._data;
           end = i.end;
           return *(this);
       }*/
       
       iterator operator == (const iterator& i){
           return *this;
       }
      
      bool operator != (const iterator& i) const {
          if(end != i.end) return true;
          else return false;
      }
   private:
       size_t        numBuckets;
       size_t        pos;
       vector<Data>* buckets;
       Data          _data;
       bool          end = false;
       
   };

   void init(size_t b) { _numBuckets = b; _buckets = new vector<Data>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<Data>& operator [] (size_t i) { return _buckets[i]; }
   const vector<Data>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const {
       Data _buf;
       int i = 0;
       while(_buckets[i].size() == 0){ i++; if(i == _numBuckets) return end();}
       return iterator(_buckets[i].at(0),0,_numBuckets,_buckets);
   }
   // Pass the end
   iterator end() const {
       Data dummy;
       iterator enditr(dummy);
       enditr.end = true;
       return enditr;
   }
   // return true if no valid data
   bool empty() const {
       int ID = 0;
       while(_buckets[ID].size() == 0){
           ID++;
           if (ID == numBuckets){
               return true;
           }
       }
       return false;
   }
   // number of valid data
   size_t size() const {
       size_t s = 0;
       for(iterator it = begin(); it != end(); it++){
           s++;
       }
       return s;
   }

   // check if d is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const Data& d) const {
       for(int i = 0;i < _buckets[bucketNum(d)].size(); i++){
           if(_buckets[bucketNum(d)][i].getName() == d.getName()){
               return true;
           }
       }
       return false;
   }

   // query if d is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(Data& d) const {
       for(int i = 0;i < _buckets[bucketNum(d)].size(); i++){
           if(_buckets[bucketNum(d)][i].getName() == d.getName()){
               d = _buckets[bucketNum(d)][i]; return true;
           }
       }
       return false;
   }

   // update the entry in hash that is equal to d (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const Data& d) {
           for(int i = 0;i < _buckets[bucketNum(d)].size(); i++){
               if(_buckets[bucketNum(d)][i] == d){
                   _buckets[bucketNum(d)][i] = d; return true;
               }
           }
       insert(d); return false;
   }

   // return true if inserted successfully (i.e. d is not in the hash)
   // return false is d is already in the hash ==> will not insert
   bool insert(const Data& d) {
       int buckid = d() % _numBuckets;
       Data _buf = d;
       if(!check(d)){
           _buckets[buckid].push_back(d);
           return true;
       }
       else return false;
   }

   // return true if removed successfully (i.e. d is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const Data& d) {
       for(int i = 0;i < _buckets[bucketNum(d)].size(); i++){
           if(_buckets[bucketNum(d)][i] == d){
               _buckets[bucketNum(d)][i] = _buckets[bucketNum(d)].back();
               _buckets[bucketNum(d)].pop_back();
               return true;
           }
       }
       return false;
   }

private:
   // Do not add any extra data member
   size_t            _numBuckets;
   vector<Data>*     _buckets;

   size_t bucketNum(const Data& d) const {
      return (d() % _numBuckets); }
};

#endif // MY_HASH_SET_H
