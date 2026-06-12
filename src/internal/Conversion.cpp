/*
 * DUNE DAQ modification notice:
 * This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
 * Fork baseline commit: dbe-02-12-17 (2022-05-12).
 * Renamed since fork: no.
 */

#include "dbe/Conversion.hpp"
#include "dbe/Exceptions.hpp"

namespace dbe {
namespace convert {

template<> QStringList to<QStringList>(std::vector<std::string> const & x) {
    QStringList ret;

    for(std::string const & a : x) {
        ret.append(QString::fromStdString(a));
    }

    return ret;
}

template<>
std::string to<std::string>(QStringList const & DataList) {
    std::string rString;

    for(int i = 0; i < DataList.size(); ++i) {
        rString = DataList.at(i).toStdString();
    }

    return rString;
}

template<>
std::vector<std::string> to<std::vector<std::string> >(QStringList const & DataList) {
    std::vector<std::string> rVector;

    for(int i = 0; i < DataList.size(); ++i) {
        rVector.push_back(DataList.at(i).toStdString());
    }

    return rVector;
}

template<>
std::string to<std::string>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    Q_UNUSED (Format)

    std::string rString;

    for(int i = 0; i < DataList.size(); ++i) {
        rString = DataList.at(i).toStdString();
    }

    return rString;
}

template<>
std::vector<std::string> to<std::vector<std::string>>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    Q_UNUSED (Format)

    std::vector<std::string> rVector;

    for(int i = 0; i < DataList.size(); ++i) {
        rVector.push_back(DataList.at(i).toStdString());
    }

    return rVector;
}

template<>
u_int8_t to<u_int8_t>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    bool ok = false;
    u_int8_t uChar = 0;

    if(DataList.size() > 0) {
        if(Format == dunedaq::conffwk::dec_int_format) {
            uChar = (u_int8_t) (DataList.at(0).toULong(&ok, 10));
        } else if(Format == dunedaq::conffwk::hex_int_format) {
            uChar = (u_int8_t) (DataList.at(0).toULong(&ok, 16));
        } else if(Format == dunedaq::conffwk::oct_int_format) {
            uChar = (u_int8_t) (DataList.at(0).toULong(&ok, 8));
        } else if(Format == dunedaq::conffwk::na_int_format) {
            uChar = (u_int8_t) (DataList.at(0).toULong(&ok));
        }
    }

    return uChar;
}

template<>
std::vector<u_int8_t> to<std::vector<u_int8_t> >(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    std::vector<u_int8_t> rVector;

    for(int i = 0; i < DataList.size(); ++i) {
        bool ok = false;
        u_int8_t uChar = 0;

        if(Format == dunedaq::conffwk::dec_int_format) {
            uChar = (u_int8_t) (DataList.at(i).toULong(&ok, 10));
        } else if(Format == dunedaq::conffwk::hex_int_format) {
            uChar = (u_int8_t) (DataList.at(i).toULong(&ok, 16));
        } else if(Format == dunedaq::conffwk::oct_int_format) {
            uChar = (u_int8_t) (DataList.at(i).toULong(&ok, 8));
        } else if(Format == dunedaq::conffwk::na_int_format) {
            uChar = (u_int8_t) (DataList.at(i).toULong(&ok));
        }

        rVector.push_back(uChar);
    }

    return rVector;
}

template<>
int8_t to<int8_t>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    bool ok = false;
    int8_t sChar = 0;

    if(DataList.size() > 0) {
        if(Format == dunedaq::conffwk::dec_int_format) {
            sChar = (int8_t) (DataList.at(0).toLong(&ok, 10));
        } else if(Format == dunedaq::conffwk::hex_int_format) {
            sChar = (int8_t) (DataList.at(0).toLong(&ok, 16));
        } else if(Format == dunedaq::conffwk::oct_int_format) {
            sChar = (int8_t) (DataList.at(0).toLong(&ok, 8));
        } else if(Format == dunedaq::conffwk::na_int_format) {
            sChar = (int8_t) (DataList.at(0).toLong(&ok));
        }
    }

    return sChar;
}

template<>
std::vector<int8_t> to<std::vector<int8_t>>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    std::vector<int8_t> rVector;

    for(int i = 0; i < DataList.size(); ++i) {
        bool ok = false;
        int8_t sChar = 0;

        if(Format == dunedaq::conffwk::dec_int_format) {
            sChar = (int8_t) (DataList.at(i).toLong(&ok, 10));
        } else if(Format == dunedaq::conffwk::hex_int_format) {
            sChar = (int8_t) (DataList.at(i).toLong(&ok, 16));
        } else if(Format == dunedaq::conffwk::oct_int_format) {
            sChar = (int8_t) (DataList.at(i).toLong(&ok, 8));
        } else if(Format == dunedaq::conffwk::na_int_format) {
            sChar = (int8_t) (DataList.at(i).toLong(&ok));
        }

        rVector.push_back(sChar);
    }

    return rVector;
}

template<>
u_int16_t to<u_int16_t>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    bool ok = false;
    u_int16_t uShort = 0;

    if(DataList.size() > 0) {
        if(Format == dunedaq::conffwk::dec_int_format) {
            uShort = DataList.at(0).toUShort(&ok, 10);
        } else if(Format == dunedaq::conffwk::hex_int_format) {
            uShort = DataList.at(0).toUShort(&ok, 16);
        } else if(Format == dunedaq::conffwk::oct_int_format) {
            uShort = DataList.at(0).toUShort(&ok, 8);
        } else if(Format == dunedaq::conffwk::na_int_format) {
            uShort = DataList.at(0).toUShort(&ok);
        }
    }

    return uShort;
}

template<>
std::vector<u_int16_t> to<std::vector<u_int16_t>>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    std::vector<u_int16_t> rVector;

    for(int i = 0; i < DataList.size(); ++i) {
        bool ok = false;
        u_int16_t uShort = 0;

        if(Format == dunedaq::conffwk::dec_int_format) {
            uShort = DataList.at(i).toUShort(&ok, 10);
        } else if(Format == dunedaq::conffwk::hex_int_format) {
            uShort = DataList.at(i).toUShort(&ok, 16);
        } else if(Format == dunedaq::conffwk::oct_int_format) {
            uShort = DataList.at(i).toUShort(&ok, 8);
        } else if(Format == dunedaq::conffwk::na_int_format) {
            uShort = DataList.at(i).toUShort(&ok);
        }

        rVector.push_back(uShort);
    }

    return rVector;
}

template<>
int16_t to<int16_t>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    bool ok = false;
    int16_t sShort = 0;

    if(DataList.size() > 0) {
        if(Format == dunedaq::conffwk::dec_int_format) {
            sShort = DataList.at(0).toShort(&ok, 10);
        } else if(Format == dunedaq::conffwk::hex_int_format) {
            sShort = DataList.at(0).toShort(&ok, 16);
        } else if(Format == dunedaq::conffwk::oct_int_format) {
            sShort = DataList.at(0).toShort(&ok, 8);
        } else if(Format == dunedaq::conffwk::na_int_format) {
            sShort = DataList.at(0).toShort(&ok);
        }
    }

    return sShort;
}

template<>
std::vector<int16_t> to<std::vector<int16_t>>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    std::vector<int16_t> rVector;

    for(int i = 0; i < DataList.size(); ++i) {
        bool ok = false;
        int16_t sShort = 0;

        if(Format == dunedaq::conffwk::dec_int_format) {
            sShort = DataList.at(i).toShort(&ok, 10);
        } else if(Format == dunedaq::conffwk::hex_int_format) {
            sShort = DataList.at(i).toShort(&ok, 16);
        } else if(Format == dunedaq::conffwk::oct_int_format) {
            sShort = DataList.at(i).toShort(&ok, 8);
        } else if(Format == dunedaq::conffwk::na_int_format) {
            sShort = DataList.at(i).toShort(&ok);
        }

        rVector.push_back(sShort);
    }

    return rVector;
}

template<>
u_int32_t to<u_int32_t>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    bool ok = false;
    u_int32_t uLong = 0;

    if(DataList.size() > 0) {
        if(Format == dunedaq::conffwk::dec_int_format) {
            uLong = DataList.at(0).toULong(&ok, 10);
        } else if(Format == dunedaq::conffwk::hex_int_format) {
            uLong = DataList.at(0).toULong(&ok, 16);
        } else if(Format == dunedaq::conffwk::oct_int_format) {
            uLong = DataList.at(0).toULong(&ok, 8);
        } else if(Format == dunedaq::conffwk::na_int_format) {
            uLong = DataList.at(0).toULong(&ok);
        }
    }

    return uLong;
}

template<>
std::vector<u_int32_t> to<std::vector<u_int32_t>>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    std::vector<u_int32_t> rVector;

    for(int i = 0; i < DataList.size(); ++i) {
        bool ok = false;
        u_int32_t uLong = 0;

        if(Format == dunedaq::conffwk::dec_int_format) {
            uLong = DataList.at(i).toULong(&ok, 10);
        } else if(Format == dunedaq::conffwk::hex_int_format) {
            uLong = DataList.at(i).toULong(&ok, 16);
        } else if(Format == dunedaq::conffwk::oct_int_format) {
            uLong = DataList.at(i).toULong(&ok, 8);
        } else if(Format == dunedaq::conffwk::na_int_format) {
            uLong = DataList.at(i).toULong(&ok);
        }

        rVector.push_back(uLong);
    }

    return rVector;
}

template<>
int32_t to<int32_t>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    bool ok = false;
    int32_t sLong = 0;

    if(DataList.size() > 0) {
        if(Format == dunedaq::conffwk::dec_int_format) {
            sLong = DataList.at(0).toLong(&ok, 10);
        } else if(Format == dunedaq::conffwk::hex_int_format) {
            sLong = DataList.at(0).toLong(&ok, 16);
        } else if(Format == dunedaq::conffwk::oct_int_format) {
            sLong = DataList.at(0).toLong(&ok, 8);
        } else if(Format == dunedaq::conffwk::na_int_format) {
            sLong = DataList.at(0).toLong(&ok);
        }
    }

    return sLong;
}

template<>
std::vector<int32_t> to<std::vector<int32_t>>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    std::vector<int32_t> rVector;

    for(int i = 0; i < DataList.size(); ++i) {
        bool ok = false;
        int32_t sLong = 0;

        if(Format == dunedaq::conffwk::dec_int_format) {
            sLong = DataList.at(i).toLong(&ok, 10);
        } else if(Format == dunedaq::conffwk::hex_int_format) {
            sLong = DataList.at(i).toLong(&ok, 16);
        } else if(Format == dunedaq::conffwk::oct_int_format) {
            sLong = DataList.at(i).toLong(&ok, 8);
        } else if(Format == dunedaq::conffwk::na_int_format) {
            sLong = DataList.at(i).toLong(&ok, 10);
        }

        rVector.push_back(sLong);
    }

    return rVector;
}

template<>
u_int64_t to<u_int64_t>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    bool ok = false;
    u_int64_t u64 = 0;

    if(DataList.size() > 0) {
        if(Format == dunedaq::conffwk::dec_int_format) {
            u64 = (uint64_t) (DataList.at(0).toULongLong(&ok, 10));
        } else if(Format == dunedaq::conffwk::hex_int_format) {
            u64 = (uint64_t) (DataList.at(0).toULongLong(&ok, 16));
        } else if(Format == dunedaq::conffwk::oct_int_format) {
            u64 = (uint64_t) (DataList.at(0).toULongLong(&ok, 8));
        } else if(Format == dunedaq::conffwk::na_int_format) {
            u64 = (uint64_t) (DataList.at(0).toULongLong(&ok, 10));
        }
    }

    return u64;
}

template<>
std::vector<u_int64_t> to<std::vector<u_int64_t>>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    std::vector<u_int64_t> rVector;

    for(int i = 0; i < DataList.size(); ++i) {
        bool ok = false;
        u_int64_t u64 = 0;

        if(Format == dunedaq::conffwk::dec_int_format) {
            u64 = (uint64_t) (DataList.at(i).toULongLong(&ok, 10));
        } else if(Format == dunedaq::conffwk::hex_int_format) {
            u64 = (uint64_t) (DataList.at(i).toULongLong(&ok, 16));
        } else if(Format == dunedaq::conffwk::oct_int_format) {
            u64 = (uint64_t) (DataList.at(i).toULongLong(&ok, 8));
        } else if(Format == dunedaq::conffwk::na_int_format) {
            u64 = (uint64_t) (DataList.at(i).toULongLong(&ok, 10));
        }

        rVector.push_back(u64);
    }

    return rVector;
}

template<>
int64_t to<int64_t>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    bool ok = false;
    int64_t i64 = 0;

    if(DataList.size() > 0) {
        if(Format == dunedaq::conffwk::dec_int_format) {
            i64 = (int64_t) (DataList.at(0).toLongLong(&ok, 10));
        } else if(Format == dunedaq::conffwk::hex_int_format) {
            i64 = (int64_t) (DataList.at(0).toLongLong(&ok, 16));
        } else if(Format == dunedaq::conffwk::oct_int_format) {
            i64 = (int64_t) (DataList.at(0).toLongLong(&ok, 8));
        } else if(Format == dunedaq::conffwk::na_int_format) {
            i64 = (int64_t) (DataList.at(0).toLongLong(&ok, 10));
        }
    }

    return i64;
}

template<>
std::vector<int64_t> to<std::vector<int64_t>>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    std::vector<int64_t> rVector;

    for(int i = 0; i < DataList.size(); ++i) {
        bool ok = false;
        int64_t i64 = 0;

        if(Format == dunedaq::conffwk::dec_int_format) {
            i64 = (int64_t) (DataList.at(i).toLongLong(&ok, 10));
        } else if(Format == dunedaq::conffwk::hex_int_format) {
            i64 = (int64_t) (DataList.at(i).toLongLong(&ok, 16));
        } else if(Format == dunedaq::conffwk::oct_int_format) {
            i64 = (int64_t) (DataList.at(i).toLongLong(&ok, 8));
        } else if(Format == dunedaq::conffwk::na_int_format) {
            i64 = (int64_t) (DataList.at(i).toLongLong(&ok, 10));
        }

        rVector.push_back(i64);
    }

    return rVector;
}

template<>
float to<float>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    Q_UNUSED (Format)

    float f {0};

    for(auto i = 0; i < DataList.size(); ++i) {
        f = DataList.at(i).toFloat();
    }

    return f;
}

template<>
std::vector<float> to<std::vector<float>>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    Q_UNUSED (Format)

    std::vector<float> rVector;

    for(int i = 0; i < DataList.size(); ++i) {
        rVector.push_back(DataList.at(i).toFloat());
    }

    return rVector;
}

template<>
double to<double>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    Q_UNUSED (Format)

    double d {0};

    for(int i = 0; i < DataList.size(); ++i) {
        d = DataList.at(i).toDouble();
    }

    return d;
}

template<>
std::vector<double> to<std::vector<double>>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    Q_UNUSED (Format)

    std::vector<double> rVector;

    for(int i = 0; i < DataList.size(); ++i) {
        rVector.push_back(DataList.at(i).toDouble());
    }

    return rVector;
}

template<>
bool to<bool>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    bool b = false;
    Q_UNUSED (Format)

    for(int i = 0; i < DataList.size(); ++i) {
        if(QString::compare(DataList.at(i), QString("true"), Qt::CaseInsensitive) == 0
                || QString::compare(DataList.at(i), QString("1"), Qt::CaseInsensitive) == 0) {
            b = true;
        } else if(QString::compare(DataList.at(i), QString("false"), Qt::CaseInsensitive) == 0
                || QString::compare(DataList.at(i), QString("0"), Qt::CaseInsensitive) == 0) {
            b = false;
        } else {
            std::string message = "Conversion to enum of " + DataList.at(i).toStdString() + " is not possible!";
            throw daq::dbe::BadConversion( ERS_HERE, message.c_str() );
        }
    }

    return b;
}

template<>
std::vector<bool> to<std::vector<bool>>(QStringList const & DataList, dunedaq::conffwk::int_format_t Format) {
    std::vector<bool> rVector;
    Q_UNUSED (Format)

    for(int i = 0; i < DataList.size(); ++i) {
        if(QString::compare(DataList.at(i), QString("true"), Qt::CaseInsensitive) == 0
                || QString::compare(DataList.at(i), QString("1"), Qt::CaseInsensitive) == 0) {
            rVector.push_back(true);
        } else if(QString::compare(DataList.at(i), QString("false"), Qt::CaseInsensitive) == 0
                || QString::compare(DataList.at(i), QString("0"), Qt::CaseInsensitive) == 0) {
            rVector.push_back(false);
        } else {
            std::string message = "Conversion to enum of " + DataList.at(i).toStdString() + " is not possible!";
            throw daq::dbe::BadConversion( ERS_HERE, message.c_str() );
        }
    }

    return rVector;
}

}
}
