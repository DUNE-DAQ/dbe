// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: yes (from dbe/config_reference.hpp to include/dbe/config_reference.hpp).

/*
 * config_reference.hpp
 *
 *  Created on: Feb 1, 2016
 *      Author: Leonidas Georgopoulos
 */

#ifndef DBE_CONFIG_REFERENCE_HPP_
#define DBE_CONFIG_REFERENCE_HPP_

#include "dbe/Exceptions.hpp"
#include "dbe/config_object_key.hpp"

#include "conffwk/ConfigObject.hpp"

#include <vector>
#include <memory>

#ifndef DONOT_THROW_ON_NULL_CONFIGOBJECT_REFERENCE
#define DEBUG_THROW_ON_NULL_CONFIGOBJECT_REFERENCE \
	if(check_null and rval.is_deleted()) \
	throw daq::dbe::null_config_reference_access(ERS_HERE);
#else
#define DEBUG_THROW_ON_NULL_CONFIGOBJECT_REFERENCE
#endif

namespace dbe
{
namespace inner
{
class dbcontroller;

namespace configobject
{

//------------------------------------------------------------------------------------------
class oref;

/**
 * A basic access control class such that only through oref it is possible to access ConfigObjects
 */
class wref
{
		dunedaq::conffwk::ConfigObject ref;
		wref(dunedaq::conffwk::ConfigObject const & o)
				: ref(o)
		{
		}

		wref(wref const &) = delete;
		wref operator=(wref const &) = delete;

		friend class oref;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
struct refhasher;

class tref;
class vref;
template<typename > class gref;
template<typename > class aref;
/**
 * Provides a consistent reference to the underlying object referenced by ConfigObject
 */
class oref
{
		friend class tref;
		friend class vref;
		friend struct refhasher;
		template<typename > friend class aref;
		template<typename > friend class gref;
		friend class dbe::inner::dbcontroller;
		friend bool operator ==(dbe::cokey const & left, dbe::cokey const & right);

		wref ref;
		std::string this_last_full_name;

		/**
		 * Permits to create a reference to an already existing configuration object
		 *
		 * @param o the configobject to wrap this around
		 */
		oref(dunedaq::conffwk::ConfigObject const & o) noexcept(true)
				: ref
					{ o },
					this_last_full_name(o.full_name())
		{
		}

		std::string full_name() const
		{
			return this_last_full_name;
		}

		void rename(std::string const & newname)
		{
			ref.ref.rename(newname);
			this_last_full_name = newname;
		}

		/**
		 * Explicit conversion for friends only returns an null ConfigObject in case that the underlying
		 * object has been deleted. Otherwise it returns the underlying ConfigObject reference,
		 * which is expected to be managed by dbe::inner::dbecontroller and be consistent or invalid.
		 */
		explicit operator dunedaq::conffwk::ConfigObject&() noexcept
		{
			return ref.ref;
		}

		explicit operator dunedaq::conffwk::ConfigObject const &() const noexcept
		{
			return ref.ref;
		}

		// non-copyable
		oref(oref const &) = delete;
		oref operator=(oref const &) = delete;

	public:
		operator dbe::cokey() const
		{
			return
			{	ref.ref.UID(),ref.ref.class_name()};
		}
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename T> class ref_interface;

template<typename T, typename U> class authorized_getter
{
	private:
		ref_interface<T> const * that;

	public:
		authorized_getter(ref_interface<T> const * caller)
				: that(caller)
		{
		}

		U operator()(std::string const & key);
};

template<typename T> class ref_interface
{
	private:
		explicit operator dunedaq::conffwk::ConfigObject &() noexcept
		{
			return static_cast<T*>(this)->ref();
		}

		explicit operator dunedaq::conffwk::ConfigObject &() const noexcept
		{
			return static_cast<T const *>(this)->ref();
		}

	protected:
		/**
		 * Retrieve values of type U referred by the given key
		 *
		 * @param key is the attribute name
		 * @return the value of type U referred by the named attribute
		 */
		template<typename U> U getdirect(std::string const & key) const
		{
			U value;
			try
			{
				static_cast<dunedaq::conffwk::ConfigObject &>(*const_cast<ref_interface<T>*>(this)).get(key, value);
			}
			catch (dunedaq::conffwk::Exception const & e)
			{
				// TODO handle retrieval errors here
			}
			return value;
		}

		template<typename X, typename U> friend class authorized_getter;

	public:

		bool is_null() const
		{
			return static_cast<dunedaq::conffwk::ConfigObject &>(static_cast<T const *>(this)->ref(false)).is_deleted();
		}

		std::string UID() const
		{
			return static_cast<dunedaq::conffwk::ConfigObject &>(*this).UID();
		}

		std::string class_name() const
		{
			return static_cast<dunedaq::conffwk::ConfigObject &>(*this).class_name();
		}

		std::string full_name() const
		{
			return static_cast<dunedaq::conffwk::ConfigObject &>(*this).full_name();
		}

		std::string contained_in() const
		{
			return static_cast<dunedaq::conffwk::ConfigObject &>(*this).contained_in();
		}

		/**
		 * Retrieve values of type U referred by the given key
		 *
		 * @param key is the attribute name
		 * @return the value of type U referred by the named attribute
		 */
		template<typename U> U get(std::string const & key)
		{
			authorized_getter<T, U> guard(this);
			return guard(key);
		}

		/**
		 * Compatibility interface to support ConfigObject style queries
		 *
		 * @param key a string of what to look for
		 * @param val is a reference to the object where the value of matching type will be placed
		 */
		template<typename U> void get(std::string const & key, U & val)
		{
			val = get<U>(key);
		}

		/**
		 * New method to retrieve reference to this object through some relation
		 *
		 * @param name of of the relation to check references through to this object
		 * @param check_composite_only enables checking of only composite parent objects
		 * @return a list of objects referencing this
		 */
		std::vector<T> referenced_by(std::string const & name = "*", bool check_composite_only =
				true) const;

		void set_obj_null(std::string const & name, bool is_simple, bool skip_non_null_check = false)
		{
			if(is_simple)
			{
			  static_cast<dunedaq::conffwk::ConfigObject &>(*this).set_obj(name, nullptr, skip_non_null_check);
			} else
			{
			  static_cast<dunedaq::conffwk::ConfigObject &>(*this).set_objs(name, {}, skip_non_null_check);
			}
		}

		void set_obj(std::string const & name, T const & other,
									bool skip_non_null_check = false)
		{
    static_cast<dunedaq::conffwk::ConfigObject &> ( *this ).set_obj ( name,
                                                    &static_cast<dunedaq::conffwk::ConfigObject &> ( other ),
																									skip_non_null_check);
		}

		void set_objs(std::string const & name, std::vector<T> const & others,
									bool skip_non_null_check = false)
		{
			std::vector<dunedaq::conffwk::ConfigObject const *> configobjects;

			std::transform(std::begin(others), std::end(others),
											std::back_inserter(configobjects), [](T const & aref )
    {
      return &static_cast<dunedaq::conffwk::ConfigObject const &> ( aref );
    } );

			static_cast<dunedaq::conffwk::ConfigObject &>(*this).set_objs(name, configobjects, skip_non_null_check);
		}

		template<typename U> void set_by_val(std::string const & name, U val)
		{
			static_cast<dunedaq::conffwk::ConfigObject &>(*this).set_by_val(name, val);
		}

		template<typename U> void set_by_ref(std::string const & name, U & val)
		{
			static_cast<dunedaq::conffwk::ConfigObject &>(*this).set_by_ref(name, val);
		}

		void set_enum(std::string const & name, std::string const & val)
		{
			static_cast<dunedaq::conffwk::ConfigObject &>(*this).set_enum(name, val);
		}

		void set_class(std::string const & name, std::string const & val)
		{
			static_cast<dunedaq::conffwk::ConfigObject &>(*this).set_class(name, val);
		}

		void set_date(std::string const & name, std::string const & val)
		{
			static_cast<dunedaq::conffwk::ConfigObject &>(*this).set_date(name, val);
		}

		void set_time(std::string const & name, std::string const & val)
		{
			static_cast<dunedaq::conffwk::ConfigObject &>(*this).set_time(name, val);
		}

		void set_enum(std::string const & name, const std::vector<std::string>& value)
		{
			static_cast<dunedaq::conffwk::ConfigObject &>(*this).set_enum(name, value);
		}

		void set_class(std::string const & name, const std::vector<std::string>& value)
		{
			static_cast<dunedaq::conffwk::ConfigObject &>(*this).set_class(name, value);
		}

		void set_date(std::string const & name, const std::vector<std::string>& value)
		{
			static_cast<dunedaq::conffwk::ConfigObject &>(*this).set_date(name, value);
		}

		void set_time(std::string const & name, const std::vector<std::string>& value)
		{
			static_cast<dunedaq::conffwk::ConfigObject &>(*this).set_time(name, value);
		}

		void move(std::string const & at)
		{
			static_cast<dunedaq::conffwk::ConfigObject &>(*this).move(at);
		}

		/**
		 *  \brief Print details of object's attributes and relationships.
		 */

		void print_ref(std::ostream& s, /*!< the output stream */
                   dunedaq::conffwk::Configuration &
                   conf, /*!< the configuration object (required to read schema description) */
										const std::string& prefix = "", /*!< optional shift output using prefix */
										bool show_contained_in = false /*!< optional print out info about object database file */
										) const
		{
			static_cast<dunedaq::conffwk::ConfigObject &>(*this).print_ref(s, conf, prefix, show_contained_in);
		}

		friend std::ostream & operator<<(std::ostream & os,
																			dbe::inner::configobject::tref const & atref);

		friend class dbe::inner::configobject::tref;
		friend class dbe::inner::configobject::vref;
		friend class dbe::inner::configobject::aref<T>;
};
//------------------------------------------------------------------------------------------

/**
 * Retrieving a tref through ref_interface should be made with a lookup
 */
template<typename T> class authorized_getter<T, configobject::tref>
{
	private:
		ref_interface<T> const * that;

	public:
		authorized_getter(ref_interface<T> const * caller)
				: that(caller)
		{
		}

		configobject::tref operator()(std::string const & key);

};

/**
 * Retrieving a tref through ref_interface should be made with a lookup
 */
template<typename T> class authorized_getter<T, std::vector<configobject::tref>>
{
	private:
		ref_interface<T> const * that;

	public:
		authorized_getter(ref_interface<T> const * caller)
				: that(caller)
		{
		}

		std::vector<configobject::tref> operator()(std::string const & key);

};

/**
 * Retrieving ConfigObject directly through ref_interface is not permitted
 */
template<typename T> class authorized_getter<T, dunedaq::conffwk::ConfigObject>
{
	private:
		ref_interface<T> const * that;

	public:
		authorized_getter(ref_interface<T> const * caller)
				: that(caller)
		{
		}

		dunedaq::conffwk::ConfigObject operator()(std::string const & key) = delete;

};

/**
 * Retrieving ConfigObject directly through ref_interface is not permitted
 */
template<typename T> class authorized_getter<T, std::vector<dunedaq::conffwk::ConfigObject>>
{
	private:
		ref_interface<T> const * that;

	public:
		authorized_getter(ref_interface<T> const * caller)
				: that(caller)
		{
		}

		std::vector<dunedaq::conffwk::ConfigObject> operator()(std::string const & key) = delete;

};

template<typename T, typename U> U authorized_getter<T, U>::operator()(
		std::string const & key)
{
	return this->that->template getdirect<U>(key);
}

//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/**
 * Provides hashing facilities for objects of type shared_ptr<oref> such that they can be inserted
 * in unordered_set / unordered_map type of containers
 */
struct refhasher
{
		size_t operator()(dbe::cokey const & o) const
		{
			return std::hash<std::string>()(o.this_class + o.this_name);
		}
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
class tref:
						public ref_interface<tref>
{
	private:
		std::shared_ptr<oref> refered;

		dunedaq::conffwk::ConfigObject & ref(bool check_null = true) const
		{
			dunedaq::conffwk::ConfigObject & rval = static_cast<dunedaq::conffwk::ConfigObject &>(*refered.get());
			DEBUG_THROW_ON_NULL_CONFIGOBJECT_REFERENCE
			return rval;
		}

		tref(std::shared_ptr<oref> other)
				: refered(other)
		{
		}

		friend class ref_interface<tref> ;
		friend class dbe::inner::dbcontroller;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
/**
 * Light weight observer pointer interface using weak pointer to avoid cyclic referencing
 */
class vref:
						public ref_interface<vref>
{
	private:
		std::weak_ptr<oref> refered;

		dunedaq::conffwk::ConfigObject & ref(bool check_null = false) const
		{
			std::shared_ptr<oref> return_val =
					refered.expired() ?
							std::shared_ptr<oref>(new oref(dunedaq::conffwk::ConfigObject())) : refered.lock();
			dunedaq::conffwk::ConfigObject & rval = static_cast<dunedaq::conffwk::ConfigObject &>(*return_val);
			DEBUG_THROW_ON_NULL_CONFIGOBJECT_REFERENCE
			return rval;
		}

		vref(std::weak_ptr<oref> other)
				: refered(other)
		{
		}

		friend class ref_interface<vref> ;
		friend class dbe::inner::dbcontroller;
};
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename T> inline std::ostream & operator<<(std::ostream & os,
																											ref_interface<T> const & atref)
{
	return os << static_cast<T const &>(atref).ref();
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------
template<typename T>
inline bool operator==(ref_interface<T> const & l,
												ref_interface<T> const & r)
{
	return l.UID() == r.UID() and l.class_name() == r.class_name();
}
//------------------------------------------------------------------------------------------

}
}
}

#endif /* DBE_CONFIG_REFERENCE_HPP_ */
