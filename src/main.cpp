#include <bits/stdc++.h>
#include "allocator.hpp"
using namespace std;

static bool is_number(const string& s){
    if(s.empty()) return false;
    size_t i=0; if(s[0]=='+'||s[0]=='-') i=1;
    for(; i<s.size(); ++i) if(!isdigit((unsigned char)s[i])) return false;
    return true;
}

int main(){
    ios::sync_with_stdio(false); cin.tie(nullptr);
    TLSFAllocator* alloc=nullptr; bool inited=false;
    string tok;
    auto do_init=[&](size_t n){ if(alloc){delete alloc; alloc=nullptr;} alloc=new TLSFAllocator(n); inited=true; };
    while (cin>>tok){
        string low=tok; for(char& c:low) c=tolower((unsigned char)c);
        if(!inited && is_number(low)){
            size_t n=0; try{ n=stoull(low);}catch(...){ n=0;} do_init(n); continue;
        }
        if(low=="#"){ string dummy; getline(cin,dummy); continue; }
        if(low=="init"||low=="initialize"||low=="pool"){ size_t n; if(!(cin>>n)) break; do_init(n); continue; }
        if(!inited) { string rest; getline(cin,rest); continue; }
        if(low=="alloc"||low=="allocate"||low=="a"||low=="malloc"||low=="m"){
            size_t sz; if(!(cin>>sz)) break; void* p=alloc->allocate(sz);
            if(!p) cout<<-1<< "\n";
            else { auto base=(unsigned char*)alloc->getMemoryPoolStart(); cout<< ( (unsigned char*)p - base ) << "\n"; }
            continue; }
        if(low=="free"||low=="dealloc"||low=="deallocate"||low=="f"){
            size_t off; if(!(cin>>off)) break; auto base=(unsigned char*)alloc->getMemoryPoolStart(); alloc->deallocate(base+off); continue; }
        if(low=="max"||low=="query"||low=="q"||low=="max_free"){ cout<<alloc->getMaxAvailableBlockSize()<< "\n"; continue; }
        if(low=="size"||low=="poolsize"){ cout<<alloc->getMemoryPoolSize()<< "\n"; continue; }
        if(low=="end"||low=="quit"||low=="exit"){ break; }
        if(is_number(low)){
            size_t sz=0; try{ sz=stoull(low);}catch(...){ sz=0;} void* p=alloc->allocate(sz);
            if(!p) cout<<-1<< "\n";
            else { auto base=(unsigned char*)alloc->getMemoryPoolStart(); cout<< ( (unsigned char*)p - base ) << "\n"; }
            continue; }
        string rest; getline(cin,rest);
    }
    delete alloc; return 0;
}

