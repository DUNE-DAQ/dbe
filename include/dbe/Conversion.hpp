// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/Conversion.h to include/dbe/Conversion.hpp).

#ifndef DBE_CONVERSION_H_
#define DBE_CONVERSION_H_

#include "dbe/Exceptions.hpp"

#include "conffwk/Schema.hpp"
#include "logging/Logging.hpp"

#include <string>
#include <vector>
#include <sstream>
#include <type_traits>

#include <QStringList>

namespace dbe
{

namespace convert
{
//------------------------------------------------------------------------------------------
/**
 * Convert  a value from a type to string (default decimal format output)
 *
 * @param the value to convert
 * @return a string representation of the value
 */
template<typename T> inline std::string valtostr ( T const & value )
{
  return std::to_string ( value );
}

template<> inline std::string valtostr<std::string> ( std::string const & x )
{
  return x;
}

template<> inline std::string valtostr<bool> ( bool const & s )
{
  std::ostringstream ss;
  ss << std::boolalpha << s;
  return ss.str();
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/**
 * Convert a value from a number type to string for different the output formats
 * @param value to conert
 * @param format is the format type , e.g. dunedaq::conffwk::int_format_t::dec_int_format for decimal
 * @return a string representation of the numeric input
 */
template<typename T, typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
inline std::string valtostr ( T const & value,
                              dunedaq::conffwk::int_format_t const format )
{
  if ( format == dunedaq::conffwk::dec_int_format )
  {
    return std::to_string ( value );
  }
  else
  {
    std::ostringstream ss;

    if ( format == dunedaq::conffwk::hex_int_format )
    {
      ss << std::hex << std::showbase << +((typename std::make_unsigned<T>::type) value);
    }
    else if ( format == dunedaq::conffwk::oct_int_format )
    {
      ss << std::oct << std::showbase << +((typename std::make_unsigned<T>::type) value);
    }
    else if ( format == dunedaq::conffwk::na_int_format )
    {
      ss << value;
      TLOG_DEBUG(1) <<  " This conversion should not happen for value " << ss.str()  ;
    }

    return ss.str();
  }
}

// Specialization for boolean (the std::make_unsigned is not defined for booleans)
template<> inline std::string valtostr<bool> ( bool const & value,
                                               dunedaq::conffwk::int_format_t const format )
{
  Q_UNUSED ( format );
  return valtostr<bool>(value);
}

// This is for floating point types
template<typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
inline std::string valtostr ( T const & value,
                              dunedaq::conffwk::int_format_t const format )
{
    Q_UNUSED ( format );
    return std::to_string(value);
}

// This is for simple strings
inline std::string valtostr ( std::string const & value,
                              dunedaq::conffwk::int_format_t const format )
{
  Q_UNUSED ( format );
  return value;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename T>
T to ( QStringList const & DataList )
{
  Q_UNUSED ( DataList )

  T Dummy;
  return Dummy;
}

template<typename T>
T to ( QStringList const & DataList, dunedaq::conffwk::int_format_t Format )
{
  Q_UNUSED ( Format )
  Q_UNUSED ( DataList )

  T Dummy;
  return Dummy;
}

template<typename T>
T to ( std::vector<std::string> const & x )
{
  return x;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/// Template specializations of to<T>

template<> QStringList to ( std::vector<std::string> const & DataList );

template<> bool to<bool> ( QStringList const & DataList, dunedaq::conffwk::int_format_t Format );
template<> std::vector<bool> to<std::vector<bool>> ( QStringList const & DataList,
                                                     dunedaq::conffwk::int_format_t Format );
template<> std::string to<std::string> ( QStringList const & DataList );
template<> std::vector<std::string> to<std::vector<std::string>> (
                                                                QStringList const & DataList );
template<> std::string to<std::string> ( QStringList const & DataList,
                                         dunedaq::conffwk::int_format_t Format );
template<> std::vector<std::string> to<std::vector<std::string>> (
                                                                QStringList const & DataList, dunedaq::conffwk::int_format_t Format );
template<> u_int8_t to<u_int8_t> ( QStringList const & DataList,
                                   dunedaq::conffwk::int_format_t Format );
template<> std::vector<u_int8_t> to<std::vector<u_int8_t>> (
                                                          QStringList const & DataList, dunedaq::conffwk::int_format_t Format );
template<> int8_t to<int8_t> ( QStringList const & DataList,
                               dunedaq::conffwk::int_format_t Format );
template<> std::vector<int8_t> to<std::vector<int8_t>> ( QStringList const & DataList,
                                                         dunedaq::conffwk::int_format_t Format );
template<> u_int16_t to<u_int16_t> ( QStringList const & DataList,
                                     dunedaq::conffwk::int_format_t Format );
template<> std::vector<u_int16_t> to<std::vector<u_int16_t>> (
                                                            QStringList const & DataList, dunedaq::conffwk::int_format_t Format );
template<> int16_t to<int16_t> ( QStringList const & DataList,
                                 dunedaq::conffwk::int_format_t Format );
template<> std::vector<int16_t> to<std::vector<int16_t>> ( QStringList const & DataList,
                                                           dunedaq::conffwk::int_format_t Format );
template<> u_int32_t to<u_int32_t> ( QStringList const & DataList,
                                     dunedaq::conffwk::int_format_t Format );
template<> std::vector<u_int32_t> to<std::vector<u_int32_t>> (
                                                            QStringList const & DataList, dunedaq::conffwk::int_format_t Format );
template<> int32_t to<int32_t> ( QStringList const & DataList,
                                 dunedaq::conffwk::int_format_t Format );
template<> std::vector<int32_t> to<std::vector<int32_t>> ( QStringList const & DataList,
                                                           dunedaq::conffwk::int_format_t Format );
template<> u_int64_t to<u_int64_t> ( QStringList const & DataList,
                                     dunedaq::conffwk::int_format_t Format );
template<> std::vector<u_int64_t> to<std::vector<u_int64_t>> (
                                                            QStringList const & DataList, dunedaq::conffwk::int_format_t Format );
template<> int64_t to<int64_t> ( QStringList const & DataList,
                                 dunedaq::conffwk::int_format_t Format );
template<> std::vector<int64_t> to<std::vector<int64_t>> ( QStringList const & DataList,
                                                           dunedaq::conffwk::int_format_t Format );
template<> float to<float> ( QStringList const & DataList,
                             dunedaq::conffwk::int_format_t Format );
template<> std::vector<float> to<std::vector<float>> ( QStringList const & DataList,
                                                       dunedaq::conffwk::int_format_t Format );
template<> double to<double> ( QStringList const & DataList,
                               dunedaq::conffwk::int_format_t Format );
template<> std::vector<double> to<std::vector<double>> ( QStringList const & DataList,
                                                         dunedaq::conffwk::int_format_t Format );
//------------------------------------------------------------------------------------------

}
}

#endif // DBE_CONVERSION_H_
