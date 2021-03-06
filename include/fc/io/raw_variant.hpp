#pragma once
#include <fc/exception/exception.hpp>
#include <fc/io/raw_fwd.hpp>
#include <fc/variant_object.hpp>
#include <fc/variant.hpp>

namespace fc { namespace raw {

    template<typename Stream>
    class variant_packer : public variant::visitor
    {
       public:
         variant_packer( Stream& _s ):s(_s){}
         virtual void handle()const { }
         virtual void handle( const int64_t& v )const
         {
            fc::raw::pack( s, v );
         }
         virtual void handle( const uint64_t& v )const
         {
            fc::raw::pack( s, v );
         }
         virtual void handle( const double& v )const 
         {
            fc::raw::pack( s, v );
         }
         virtual void handle( const bool& v )const
         {
            fc::raw::pack( s, v );
         }
         virtual void handle( const string& v )const
         {
            fc::raw::pack( s, v );
         }
         virtual void handle( const variant_object& v)const
         {
            fc::raw::pack( s, v );
         }
         virtual void handle( const variants& v)const
         {
            fc::raw::pack( s, v );
         }
         virtual void handle( const time_point& v)const
         {
            fc::raw::pack( s, v );
         }
         virtual void handle( const __int128& v)const
         {
            fc::raw::pack( s, v );
         }
         virtual void handle( const unsigned __int128& v)const
         {
            fc::raw::pack( s, v);
         }
        
         Stream& s;
        
    };


    template<typename Stream> 
    inline void pack( Stream& s, const variant& v )
    {
       pack( s, uint8_t(v.get_type()) );
       v.visit( variant_packer<Stream>(s) );
    }
    template<typename Stream> 
    inline void unpack( Stream& s, variant& v )
    {
      uint8_t t;
      unpack( s, t );
      const variant::type_id type = static_cast<variant::type_id>(t);
      switch( type )
      {
         case variant::type_id::null_type:
            return;
         case variant::type_id::int64_type:
         {
            int64_t val;
            raw::unpack(s,val);
            v = val;
            return;
         }
         case variant::type_id::uint64_type:
         {
            uint64_t val;
            raw::unpack(s,val);
            v = val;
            return;
         }
         case variant::type_id::double_type:
         {
            double val;
            raw::unpack(s,val);
            v = val;
            return;
         }
         case variant::type_id::bool_type:
         {
            bool val;
            raw::unpack(s,val);
            v = val;
            return;
         }
         case variant::type_id::string_type:
         {
            fc::string val;
            raw::unpack(s,val);
            v = fc::move(val);
            return;
         }
         case variant::type_id::array_type:
         {
            variants val;
            raw::unpack(s,val);
            v = fc::move(val);
            return;
         }
         case variant::type_id::object_type:
         {
            variant_object val; 
            raw::unpack(s,val);
            v = fc::move(val);
            return;
         }
         default:
            FC_THROW_EXCEPTION( parse_error_exception, "Unknown Variant Type ${t}", ("t", t) );
      }
    }

    template<typename Stream> 
    inline void pack( Stream& s, const variant_object& v ) 
    {
       unsigned_int vs = (uint32_t)v.size();
       pack( s, vs );
       for( auto itr = v.begin(); itr != v.end(); ++itr )
       {
          pack( s, itr->key() );
          pack( s, itr->value() );
       }
    }
    template<typename Stream> 
    inline void unpack( Stream& s, variant_object& v ) 
    {
       unsigned_int vs;
       unpack( s, vs );

       mutable_variant_object mvo;
       mvo.reserve(vs.value);
       for( uint32_t i = 0; i < vs.value; ++i )
       {
          fc::string key;
          fc::variant value;
          fc::raw::unpack(s,key);
          fc::raw::unpack(s,value);
          mvo.set( fc::move(key), fc::move(value) );
       }
       v = fc::move(mvo);
    }

} } // fc::raw
