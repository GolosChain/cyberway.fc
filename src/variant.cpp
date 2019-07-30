#include <algorithm>
#include <string>
#include <limits>

#include <boost/scoped_array.hpp>

#include <fc/variant.hpp>
#include <fc/variant_object.hpp>
#include <fc/exception/exception.hpp>
#include <fc/crypto/base64.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/io/json.hpp>

namespace {
    const double DOUBLE_ACCURACY = 0.0000001;
}

#ifdef __APPLE__
#   if __apple_build_version__ < 9020039 || __clang_major__ == 4   // 902.00.39 is clang 5, not actually checked, is it works there
#       define ABSENT__INT128_TYPEID
#   endif
#endif

template<typename A>
inline __int128 lexical_cast_128(const A& arg) {
#ifdef ABSENT__INT128_TYPEID
    __int128 result = 0;
    if (!boost::conversion::detail::try_lexical_convert(arg, result)) {
        boost::throw_exception(boost::bad_lexical_cast());  // don't pass typeid(__int128), which prevents linking
    }
    return result;
#else
    return boost::lexical_cast<__int128>(arg);
#endif
}


namespace fc {

variant::variant( uint8_t val )
{
    type_ = type_id::uint64_type;
    value_.as_uint64 = val;
}

variant::variant( int8_t val )
{
    type_ = type_id::int64_type;
    value_.as_int64 = val;
}

variant::variant( uint16_t val )
{
    type_ = type_id::uint64_type;
    value_.as_uint64 = val;
}

variant::variant( int16_t val )
{
    type_ = type_id::int64_type;
    value_.as_int64 = val;
}

variant::variant( uint32_t val )
{
    type_ = type_id::uint64_type;
    value_.as_uint64 = val;
}

variant::variant( int32_t val )
{
    type_ = type_id::int64_type;
    value_.as_int64 = val;
}

variant::variant( uint64_t val )
{
    type_ = type_id::uint64_type;
    value_.as_uint64 = val;
}

variant::variant( int64_t val )
{
    type_ = type_id::int64_type;
    value_.as_int64 = val;
}

variant::variant( float val )
{
    type_ = type_id::double_type;
    value_.as_double = val;
}

variant::variant( double val )
{
    type_ = type_id::double_type;
    value_.as_double = val;
}

variant::variant( bool val )
{
    type_ = type_id::bool_type;
    value_.as_bool = val;
}

variant::variant( char* str )
{
    type_ = type_id::string_type;
    value_.as_string = new std::string(str);
}

variant::variant( const char* str )
{
    type_ = type_id::string_type;
    value_.as_string = new std::string(str);;
}

variant::variant( fc::string val )
{
    type_ = type_id::string_type;
    value_.as_string = new std::string(std::move(val));
}
variant::variant( blob val )
{
    type_ = type_id::blob_type;
    value_.as_blob = new blob(std::move(val));
}

variant::variant( variant_object obj)
{
    type_ = type_id::object_type;
    value_.as_object = new variant_object(std::move(obj));
}

variant::variant( mutable_variant_object obj)
{
    type_ = type_id::object_type;
    value_.as_object = new variant_object(std::move(obj));
}

variant::variant( variants arr )
{
    type_ = type_id::array_type;
    value_.as_array = new variants(std::move(arr));
}

variant::variant(const time_point& time)
{
    type_ = type_id::time_type;
    value_.as_time = time;
}

variant::variant(const time_point_sec& time)
{
    type_ = type_id::time_type;
    value_.as_time = time;
}

variant::variant(const __int128& val)
{
    type_ = type_id::int128_type;
    value_.as_int128 = val;
}

variant::variant(const __uint128& val)
{
    type_ = type_id::uint128_type;
    value_.as_uint128 = val;
}

variant::variant( const variant& v )
{
    *this = v;
}

void variant::clear()
{
    switch (type_) {
        case type_id::array_type: delete value_.as_array; break;
        case type_id::blob_type: delete value_.as_blob; break;
        case type_id::object_type: delete value_.as_object; break;
        case type_id::string_type: delete value_.as_string; break;
        default: break;
    }
    type_ = type_id::null_type;

}

variant& variant::operator=( const variant& v )
{
   if( this == &v )
      return *this;

   clear();
   type_ = v.type_;
   switch( v.get_type() )
   {
        case type_id::object_type:
            value_.as_object = new variant_object(*v.value_.as_object);
            break;
        case type_id::array_type:
            value_.as_array = new variants(*v.value_.as_array);
            break;
        case type_id::string_type:
            value_.as_string = new std::string(*v.value_.as_string);
            break;
        case type_id::blob_type:
            value_.as_blob = new blob(*v.value_.as_blob);
            break;
        default:
            value_.as_int128 = v.value_.as_int128;
   }
   return *this;
}

void  variant::visit( const visitor& v )const
{
   switch( get_type() )
   {
      case type_id::null_type:
         v.handle();
         return;
      case type_id::int64_type:
         v.handle(value_.as_int64);
         return;
      case type_id::uint64_type:
         v.handle(value_.as_uint64);
         return;
      case type_id::double_type:
         v.handle(value_.as_double);
         return;
      case type_id::bool_type:
         v.handle(value_.as_bool);
         return;
      case type_id::string_type:
         v.handle(*value_.as_string);
         return;
       case type_id::blob_type:
         v.handle(as_string());
      case type_id::time_type:
         v.handle(value_.as_time);
         return;
      case type_id::array_type:
         v.handle(*value_.as_array);
         return;
      case type_id::object_type:
         v.handle(*value_.as_object);
         return;
      case type_id::int128_type:
         v.handle(value_.as_int128);
         return;
      case type_id::uint128_type:
         v.handle(value_.as_uint128);
         return;
      default:
         FC_THROW_EXCEPTION( assert_exception, "Invalid Type / Corrupted Memory" );
   }
}

bool variant::is_string()const
{
   return get_type() == type_id::string_type || get_type() == type_id::blob_type;
}

bool variant::is_integer()const
{
   switch( get_type() )
   {
      case type_id::int64_type:
      case type_id::uint64_type:
      case type_id::int128_type:
      case type_id::uint128_type:
      case type_id::bool_type:
         return true;
      default:
         return false;
   }
   return false;
}
bool variant::is_numeric()const
{
   switch( get_type() )
   {
      case type_id::int64_type:
      case type_id::uint64_type:
      case type_id::int128_type:
      case type_id::uint128_type:
      case type_id::double_type:
      case type_id::bool_type:
         return true;
      default:
         return false;
   }
   return false;
}

bool variant::is_blob()const
{
    return get_type() == type_id::blob_type || get_type() == type_id::string_type;
}

int64_t variant::as_int64() const try {
    return to_uint64();
} catch (const bad_cast_exception&) {
    FC_THROW_EXCEPTION( bad_cast_exception, "Invalid cast from ${type} to int64", ("type", get_type()) );
}

uint64_t variant::as_uint64() const try {
    return to_uint64();
} catch (const bad_cast_exception&) {
    FC_THROW_EXCEPTION( bad_cast_exception, "Invalid cast from ${type} to uint64", ("type", get_type()) );
}

uint64_t variant::to_uint64() const {
    switch( get_type() )
    {
       case type_id::string_type:
           return fc::to_uint64(*value_.as_string);
       case type_id::double_type:
           return int64_t(value_.as_double);
       case type_id::int64_type:
       case type_id::uint64_type:
           return value_.as_int64;
       case type_id::bool_type:
           return value_.as_bool;
       case type_id::null_type:
           return 0;
       case type_id::uint128_type:
       case type_id::int128_type:
           return uint128_to_uint64();
       default:
          throw bad_cast_exception();
    }
}

uint64_t variant::uint128_to_uint64() const {
    if (value_.as_uint128 > std::numeric_limits<uint64_t>::max()) {
        throw bad_cast_exception();
    }
    return value_.as_uint128;
}

double  variant::as_double()const
{
   switch( get_type() )
   {
      case type_id::string_type:
          return to_double(*value_.as_string);
      case type_id::double_type:
          return value_.as_double;
      case type_id::int64_type:
          return static_cast<double>(value_.as_int64);
      case type_id::uint64_type:
          return static_cast<double>(value_.as_uint64);
      case type_id::bool_type:
          return static_cast<double>(value_.as_bool);
      case type_id::null_type:
          return 0;
      case type_id::uint128_type:
      case type_id::int128_type:
          return uint128_to_double();
      default:
         FC_THROW_EXCEPTION( bad_cast_exception, "Invalid cast from ${type} to double", ("type",get_type()) );
   }
}

double variant::uint128_to_double() const {
    const __uint128 val = value_.as_uint128;
    if (val > std::numeric_limits<uint64_t>::max()) {
        FC_THROW_EXCEPTION( bad_cast_exception, "Invalid cast from ${type} to double", ("type", get_type()));
    }
    return static_cast<double>(val);
}

bool variant::as_bool() const
{
   switch( get_type() )
   {
      case type_id::string_type:
      {
          if( *value_.as_string == "true" )
             return true;
          if( *value_.as_string == "false" )
             return false;
          FC_THROW_EXCEPTION( bad_cast_exception, "Cannot convert string to bool (only \"true\" or \"false\" can be converted)" );
      }
      case type_id::double_type:
          return std::abs(value_.as_double - 0.0) < DOUBLE_ACCURACY;
      case type_id::int64_type:
      case type_id::uint64_type:
          return value_.as_int64 != 0;
      case type_id::bool_type:
          return value_.as_bool;
      case type_id::null_type:
          return false;
      case type_id::uint128_type:
      case type_id::int128_type:
          return value_.as_int128 != 0;
      default:
         FC_THROW_EXCEPTION( bad_cast_exception, "Invalid cast from ${type} to bool" , ("type",get_type()));
   }
}

string variant::as_string() const
{
   switch( get_type() )
   {
      case type_id::string_type:
          return *value_.as_string;
      case type_id::double_type:
          return to_string(value_.as_double);
      case type_id::int64_type:
          return to_string(value_.as_int64);
      case type_id::uint64_type:
          return to_string(value_.as_uint64);
      case type_id::bool_type:
          return value_.as_bool ? "true" : "false";
      case type_id::blob_type:
          if(!value_.as_blob->data.empty())
             return to_hex(value_.as_blob->data);
             //return base64_encode(value_.as_blob->data) + "=";
          return string();
      case type_id::null_type:
          return string();
      case type_id::time_type:
           return static_cast<std::string>(value_.as_time);
      case type_id::int128_type:
           return boost::lexical_cast<std::string>(value_.as_int128);
      case type_id::uint128_type:
           return boost::lexical_cast<std::string>(value_.as_uint128);

      default:
      FC_THROW_EXCEPTION( bad_cast_exception, "Invalid cast from ${type} to string", ("type", get_type() ) );
   }
}

blob& variant::get_mutable_blob()
{
  if( get_type() == type_id::string_type ) {
     auto v = variant(as_blob());
     this->operator=(fc::move(v));
  }
  if( get_type() == type_id::blob_type )
     return *value_.as_blob;

  FC_THROW_EXCEPTION( bad_cast_exception, "Invalid cast from ${type} to Blob", ("type",get_type()) );
}

const blob& variant::get_blob() const
{
    return const_cast<variant*>(this)->get_mutable_blob();
}

blob variant::as_blob() const
{
   switch( get_type() )
   {
      case type_id::null_type: return blob();
      case type_id::blob_type: return *value_.as_blob;
      case type_id::string_type:
      {
         if( value_.as_string->empty()) return blob();
//         if( value_.as_string->back() == '=' )
//         {
//            const std::string b64 = base64_decode(*value_.as_string);
//            return blob({std::vector<char>(b64.begin(), b64.end())});
//         }
         if (value_.as_string->size() % 2 == 0) try {
             blob b;
             b.data.resize(value_.as_string->size() / 2);
             from_hex(*value_.as_string, b.data.data(), b.data.size());
             return b;
         } catch(...) {
             // skip
         }
         return blob( { std::vector<char>( value_.as_string->begin(), value_.as_string->end() ) } );
      }
      default:
      case type_id::array_type:
         FC_THROW_EXCEPTION( bad_cast_exception, "Invalid cast from ${type} to Blob", ("type",get_type()) );
   }
}

time_point variant::as_time_point() const {
    if (get_type() == type_id::time_type) {
        return value_.as_time;
    } else if(get_type() == type_id::string_type) {
        return fc::time_point::from_iso_string(*value_.as_string);
    }
    FC_THROW_EXCEPTION( bad_cast_exception, "Invalid cast from ${type} to Time Point", ("type", get_type()) );
}

time_point_sec variant::as_time_point_sec() const {
   return as_time_point();
}

__uint128_t variant::as_uint128() const {
    try {
        return to_uint128();
    } catch (...) {
        FC_THROW_EXCEPTION( bad_cast_exception, "Cannot convert variant of type '${type}' into a uint128", ("type", get_type()) );
    }
}

__int128 variant::as_int128() const {
    try {
        return to_uint128();
    } catch (...) {
        FC_THROW_EXCEPTION( bad_cast_exception, "Cannot convert variant of type '${type}' into a int128", ("type", get_type()) );
    }
}

__uint128 variant::to_uint128() const {
    if( is_uint128() || is_int128()) {
        return value_.as_uint128;
    } else if (is_string()) {
       return lexical_cast_128(value_.as_string);
    } else {
       return as_uint64();
    }
}

/// @throw if get_type() != array_type
const variants& variant::get_array() const {
    return const_cast<variant*>(this)->get_mutable_array();
}

const variant& variant::operator[]( const char* key )const
{
    return get_object()[key];
}
const variant& variant::operator[]( size_t pos )const
{
    return get_array()[pos];
}
        /// @pre is_array()
size_t            variant::size()const
{
    return get_array().size();
}

variants& variant::get_mutable_array() {
    if( get_type() == type_id::array_type )
       return *value_.as_array;
    FC_THROW_EXCEPTION( bad_cast_exception, "Invalid cast from ${type} to an Array", ("type",get_type()) );
}

string& variant::get_mutable_string() {
   if( get_type() == type_id::blob_type ) {
       auto v = variant(as_string());
       this->operator=(fc::move(v));
   }
   if( get_type() == type_id::string_type )
      return *value_.as_string;
   FC_THROW_EXCEPTION( bad_cast_exception, "Invalid cast from type '${type}' to string", ("type",get_type()) );
}

const string& variant::get_string() const {
   return const_cast<variant*>(this)->get_mutable_string();
}

/// @throw if get_type() != object_type
const variant_object&  variant::get_object() const {
  if( get_type() == type_id::object_type )
     return *value_.as_object;
  FC_THROW_EXCEPTION( bad_cast_exception, "Invalid cast from type '${type}' to Object", ("type",get_type()) );
}

bool variant::has_value(const variant& v) const {
    if( v.is_null() )
        return true;

    if( get_type() != v.get_type() )
       return false;

    if( is_double() )  return std::abs(as_double() - v.as_double()) < DOUBLE_ACCURACY;
    if( is_int64() )   return as_int64()      == v.as_int64();
    if( is_uint64() )  return as_uint64()     == v.as_uint64();
    if( is_int128() )  return as_int128()     == v.as_int128();
    if( is_uint128() ) return as_uint128()    == v.as_uint128();
    if( is_time() )    return as_time_point() == v.as_time_point();
    if( is_array() )   return get_array()     == v.get_array();
    if( is_bool() )    return as_bool()       == v.as_bool();

    if( is_object() )  return get_object().has_value(v.get_object());

    if( get_type() == type_id::string_type )  return get_string() == v.get_string();
    if( get_type() == type_id::blob_type )    return get_blob()   == v.get_blob();

    return false;
}

void from_variant( const variant& var,  variants& vo )
{
   vo = var.get_array();
}

//void from_variant( const variant& var,  variant_object& vo )
//{
//   vo  = var.get_object();
//}

void from_variant( const variant& var,  variant& vo ) { vo = var; }

void to_variant( const uint8_t& var,  variant& vo )  { vo = uint64_t(var); }
// TODO: warn on overflow?
void from_variant( const variant& var,  uint8_t& vo ){ vo = static_cast<uint8_t>(var.as_uint64()); }

void to_variant( const int8_t& var,  variant& vo )  { vo = int64_t(var); }
// TODO: warn on overflow?
void from_variant( const variant& var,  int8_t& vo ){ vo = static_cast<int8_t>(var.as_int64()); }

void to_variant( const uint16_t& var,  variant& vo )  { vo = uint64_t(var); }
// TODO: warn on overflow?
void from_variant( const variant& var,  uint16_t& vo ){ vo = static_cast<uint16_t>(var.as_uint64()); }

void to_variant( const int16_t& var,  variant& vo )  { vo = int64_t(var); }
// TODO: warn on overflow?
void from_variant( const variant& var,  int16_t& vo ){ vo = static_cast<int16_t>(var.as_int64()); }

void to_variant( const uint32_t& var,  variant& vo )  { vo = uint64_t(var); }
void from_variant( const variant& var,  uint32_t& vo )
{
   vo = static_cast<uint32_t>(var.as_uint64());
}

void to_variant( const int32_t& var,  variant& vo )  {
   vo = int64_t(var);
}

void from_variant( const variant& var,  int32_t& vo )
{
   vo = static_cast<int32_t>(var.as_int64());
}

void to_variant( const unsigned __int128& var,  variant& vo ) {
   vo = var;
}

void from_variant( const variant& var,  unsigned __int128& vo )
{
   vo = var.as_uint128();
}

void to_variant( const __int128& var,  variant& vo )  {
   vo = var;
}

void from_variant( const variant& var,  __int128& vo )
{
   vo = var.as_int128();
}

void from_variant( const variant& var,  int64_t& vo )
{
   vo = var.as_int64();
}

void from_variant( const variant& var,  uint64_t& vo )
{
   vo = var.as_uint64();
}

void from_variant( const variant& var,  bool& vo )
{
   vo = var.as_bool();
}

void from_variant( const variant& var,  double& vo )
{
   vo = var.as_double();
}

void from_variant( const variant& var,  float& vo )
{
   vo = static_cast<float>(var.as_double());
}

void to_variant( const std::string& s, variant& v )
{
   v = variant( fc::string(s) );
}

void from_variant( const variant& var,  string& vo )
{
   vo = var.as_string();
}

void to_variant( const std::vector<char>& var,  variant& vo )
{
//   FC_ASSERT( var.size() <= MAX_SIZE_OF_BYTE_ARRAYS );
//   if( var.size() )
//      vo = variant(to_hex(var.data(),var.size()));
//   else vo = "";
  if( var.size() )
      // vo = variant(to_hex(var.data(),var.size()));
      vo = variant(blob{var});
  else vo = "";
}
void from_variant( const variant& var,  std::vector<char>& vo )
{
   vo = var.as_blob().data;
//   const auto& str = var.get_string();
//   FC_ASSERT( str.size() <= 2*MAX_SIZE_OF_BYTE_ARRAYS ); // Doubled because hex strings needs two characters per byte
//   FC_ASSERT( str.size() % 2 == 0, "the length of hex string should be even number" );
//   vo.resize( str.size() / 2 );
//   if( vo.size() ) {
//      size_t r = from_hex( str, vo.data(), vo.size() );
//      FC_ASSERT( r == vo.size() );
//   }
}

void to_variant( const UInt<8>& n, variant& v ) { v = uint64_t(n); }
// TODO: warn on overflow?
void from_variant( const variant& v, UInt<8>& n ) { n = static_cast<uint8_t>(v.as_uint64()); }

void to_variant( const UInt<16>& n, variant& v ) { v = uint64_t(n); }
// TODO: warn on overflow?
void from_variant( const variant& v, UInt<16>& n ) { n = static_cast<uint16_t>(v.as_uint64()); }

void to_variant( const UInt<32>& n, variant& v ) { v = uint64_t(n); }
// TODO: warn on overflow?
void from_variant( const variant& v, UInt<32>& n ) { n = static_cast<uint32_t>(v.as_uint64()); }

void to_variant( const UInt<64>& n, variant& v ) { v = uint64_t(n); }
void from_variant( const variant& v, UInt<64>& n ) { n = v.as_uint64(); }

constexpr size_t minimize_max_size = 1024;
constexpr size_t minimize_sub_max_size = minimize_max_size / 4;

string format_string( const string& frmt, const variant_object& args, bool minimize )
{
   std::string result;
   const string& format = ( minimize && frmt.size() > minimize_max_size ) ?
         frmt.substr( 0, minimize_max_size ) + "..." : frmt;
   result.reserve( minimize_sub_max_size );
   size_t prev = 0;
   size_t next = format.find( '$' );
   while( prev != string::npos && prev < format.size() ) {
      if( next != string::npos ) {
         result += format.substr( prev, next - prev );
      } else {
         result += format.substr( prev );
      }

      // if we got to the end, return it.
      if( next == string::npos ) {
         return result;
      } else if( minimize && result.size() > minimize_max_size ) {
         result += "...";
         return result;
      }

      // if we are not at the end, then update the start
      prev = next + 1;

      if( format[prev] == '{' ) {
         // if the next char is a open, then find close
         next = format.find( '}', prev );
         // if we found close...
         if( next != string::npos ) {
            // the key is between prev and next
            string key = format.substr( prev + 1, (next - prev - 1) );

            auto val = args.find( key );
            bool replaced = true;
            if( val != args.end() ) {
               if( val->value().is_object() || val->value().is_array() ) {
                  if( minimize ) {
                     replaced = false;
                  } else {
                     result += json::to_string( val->value() );
                  }
               } else if( val->value().is_blob() ) {
                  if( minimize && val->value().get_blob().data.size() > minimize_sub_max_size ) {
                     replaced = false;
                  } else {
                     result += val->value().as_string();
                  }
               } else if( val->value().is_string() ) {
                  if( minimize && val->value().get_string().size() > minimize_sub_max_size ) {
                     auto sz = std::min( minimize_sub_max_size, minimize_max_size - result.size() );
                     result += val->value().get_string().substr( 0, sz );
                     result += "...";
                  } else {
                     result += val->value().get_string();
                  }
               } else {
                  result += val->value().as_string();
               }
            } else {
               replaced = false;
            }
            if( !replaced ) {
               result += "${";
               result += key;
               result += "}";
            }
            prev = next + 1;
            // find the next $
            next = format.find( '$', prev );
         } else {
            // we didn't find it.. continue to while...
         }
      } else {
         result += format[prev];
         ++prev;
         next = format.find( '$', prev );
      }
   }
   return result;
}

   #ifdef __APPLE__
   #elif !defined(_MSC_VER)
   void to_variant( long long int s, variant& v ) { v = variant( int64_t(s) ); }
   void to_variant( unsigned long long int s, variant& v ) { v = variant( uint64_t(s)); }
   #endif

   bool operator == ( const variant& a, const variant& b )
   {
      if( a.is_string()  || b.is_string() )  return a.as_string() == b.as_string();
      if( a.is_double()  || b.is_double() )  return std::abs(a.as_double() - b.as_double()) < DOUBLE_ACCURACY;
      if( a.is_int64()   || b.is_int64() )   return a.as_int64() == b.as_int64();
      if( a.is_uint64()  || b.is_uint64() )  return a.as_uint64() == b.as_uint64();
      if( a.is_int128()  || b.is_int128() )  return a.as_int128() == b.as_int128();
      if( a.is_uint128() || b.is_uint128() ) return a.as_uint128() == b.as_uint128();
      if( a.is_time()    || b.is_time() )    return a.as_time_point() == b.as_time_point();
      if( a.is_array()   || b.is_array() )   return a.get_array() == b.get_array();
      if( a.is_bool()    || b.is_bool() )    return a.as_bool() == b.as_bool();
      if( a.is_object()  && b.is_object() )  return a.get_object() == b.get_object();
      return false;
   }

   bool operator != ( const variant& a, const variant& b )
   {
      return !( a == b );
   }

   bool operator ! ( const variant& a )
   {
      return !a.as_bool();
   }

   bool operator < ( const variant& a, const variant& b )
   {
      if( a.is_string()  || b.is_string() ) return a.as_string() < b.as_string();
      if( a.is_double()  || b.is_double() ) return a.as_double() < b.as_double();
      if( a.is_int64()   || b.is_int64() )  return a.as_int64() < b.as_int64();
      if( a.is_uint64()  || b.is_uint64() ) return a.as_uint64() < b.as_uint64();
      FC_ASSERT( false, "Invalid operation" );
   }

   bool operator > ( const variant& a, const variant& b )
   {
      if( a.is_string()  || b.is_string() ) return a.as_string() > b.as_string();
      if( a.is_double()  || b.is_double() ) return a.as_double() > b.as_double();
      if( a.is_int64()   || b.is_int64() )  return a.as_int64() > b.as_int64();
      if( a.is_uint64()  || b.is_uint64() ) return a.as_uint64() > b.as_uint64();
      FC_ASSERT( false, "Invalid operation" );
   }

   bool operator <= ( const variant& a, const variant& b )
   {
      if( a.is_string()  || b.is_string() ) return a.as_string() <= b.as_string();
      if( a.is_double()  || b.is_double() ) return a.as_double() <= b.as_double();
      if( a.is_int64()   || b.is_int64() )  return a.as_int64() <= b.as_int64();
      if( a.is_uint64()  || b.is_uint64() ) return a.as_uint64() <= b.as_uint64();
      FC_ASSERT( false, "Invalid operation" );
   }


   variant operator + ( const variant& a, const variant& b )
   {
      if( a.is_array()  && b.is_array() )
      {
         const variants& aa = a.get_array();
         const variants& ba = b.get_array();
         variants result;
         result.reserve( std::max(aa.size(),ba.size()) );
         auto num = std::max(aa.size(),ba.size());
         for( unsigned i = 0; i < num; ++i )
         {
            if( aa.size() > i && ba.size() > i )
               result[i]  = aa[i] + ba[i];
            else if( aa.size() > i )
               result[i]  = aa[i];
            else
               result[i]  = ba[i];
         }
         return result;
      }
      if( a.is_string()  || b.is_string() ) return a.as_string() + b.as_string();
      if( a.is_double()  || b.is_double() ) return a.as_double() + b.as_double();
      if( a.is_int64()   || b.is_int64() )  return a.as_int64() + b.as_int64();
      if( a.is_uint64()  || b.is_uint64() ) return a.as_uint64() + b.as_uint64();
      FC_ASSERT( false, "invalid operation ${a} + ${b}", ("a",a)("b",b) );
   }

   variant operator - ( const variant& a, const variant& b )
   {
      if( a.is_array()  && b.is_array() )
      {
         const variants& aa = a.get_array();
         const variants& ba = b.get_array();
         variants result;
         result.reserve( std::max(aa.size(),ba.size()) );
         auto num = std::max(aa.size(),ba.size());
         for( unsigned i = 0; i < num; --i )
         {
            if( aa.size() > i && ba.size() > i )
               result[i]  = aa[i] - ba[i];
            else if( aa.size() > i )
               result[i]  = aa[i];
            else
               result[i]  = ba[i];
         }
         return result;
      }
      if( a.is_string()  || b.is_string() ) return a.as_string() - b.as_string();
      if( a.is_double()  || b.is_double() ) return a.as_double() - b.as_double();
      if( a.is_int64()   || b.is_int64() )  return a.as_int64() - b.as_int64();
      if( a.is_uint64()  || b.is_uint64() ) return a.as_uint64() - b.as_uint64();
      FC_ASSERT( false, "invalid operation ${a} + ${b}", ("a",a)("b",b) );
   }
   variant operator * ( const variant& a, const variant& b )
   {
      if( a.is_double()  || b.is_double() ) return a.as_double() * b.as_double();
      if( a.is_int64()   || b.is_int64() )  return a.as_int64() * b.as_int64();
      if( a.is_uint64()  || b.is_uint64() ) return a.as_uint64() * b.as_uint64();
      if( a.is_array()  && b.is_array() )
      {
         const variants& aa = a.get_array();
         const variants& ba = b.get_array();
         variants result;
         result.reserve( std::max(aa.size(),ba.size()) );
         auto num = std::max(aa.size(),ba.size());
         for( unsigned i = 0; i < num; ++i )
         {
            if( aa.size() > i && ba.size() > i )
               result[i]  = aa[i] * ba[i];
            else if( aa.size() > i )
               result[i]  = aa[i];
            else
               result[i]  = ba[i];
         }
         return result;
      }
      FC_ASSERT( false, "invalid operation ${a} * ${b}", ("a",a)("b",b) );
   }
   variant operator / ( const variant& a, const variant& b )
   {
      if( a.is_double()  || b.is_double() ) return a.as_double() / b.as_double();
      if( a.is_int64()   || b.is_int64() )  return a.as_int64() / b.as_int64();
      if( a.is_uint64()  || b.is_uint64() ) return a.as_uint64() / b.as_uint64();
      if( a.is_array()  && b.is_array() )
      {
         const variants& aa = a.get_array();
         const variants& ba = b.get_array();
         variants result;
         result.reserve( std::max(aa.size(),ba.size()) );
         auto num = std::max(aa.size(),ba.size());
         for( unsigned i = 0; i < num; ++i )
         {
            if( aa.size() > i && ba.size() > i )
               result[i]  = aa[i] / ba[i];
            else if( aa.size() > i )
               result[i]  = aa[i];
            else
               result[i]  = ba[i];
         }
         return result;
      }
      FC_ASSERT( false, "invalid operation ${a} / ${b}", ("a",a)("b",b) );
   }
} // namespace fc
