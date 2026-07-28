// DUNE DAQ modification notice:
// This file has been modified from the original ATLAS dbe source for the DUNE DAQ project.
// Fork baseline commit: dbe-02-12-17 (2022-05-12).
// Renamed since fork: no.

/*
 * configuration.cpp
 *
 *  Created on: Mar 22, 2016
 *      Author: Leonidas Georgopoulos
 */

#include "dbe/config_ui_info.hpp"
#include "dbe/config_api_info.hpp"
#include "dbe/GraphicalClass.hpp"
#include "dbe/messenger.hpp"
#include "dbe/Conversion.hpp"

#include "dbe/config_api_get.hpp"
#include "dbe/config_api_graph.hpp"

#include <QStringList>

#include <map>
#include <vector>

using namespace dunedaq::conffwk;

dbe::ui::config::info::info ( std::vector<std::string> const & full_file_name )
  : this_full_filenames ( full_file_name )
{
  parse();
}

dbe::GraphicalClass dbe::ui::config::info::graphical ( std::string const & name ) const
{
  std::map<std::string, GraphicalClass>::const_iterator it = this_graphical.find ( name );

  if ( it != this_graphical.end() )
  {
    return it->second;
  }
  else
  {
    return
      {};
  }
}

dbe::ViewConfiguration dbe::ui::config::info::view ( std::string const & name ) const
{
  std::map<std::string, ViewConfiguration>::const_iterator it = this_views.find ( name );

  if ( it != this_views.end() )
  {
    return it->second;
  }
  else
  {
    return
      {};
  }
}

dbe::Window dbe::ui::config::info::window ( std::string const & name ) const
{
  std::map<std::string, Window>::const_iterator it = this_windows.find ( name );

  if ( it != this_windows.end() )
  {
    return it->second;
  }
  else
  {
    return
      {};
  }
}

void dbe::ui::config::info::parse()
{
  for ( std::string const & fname : this_full_filenames )
  {
    std::string Implementation = "oksconflibs:" + fname;

    try
    {
      std::shared_ptr<Configuration> ConfigFile ( new Configuration ( Implementation ) );
      std::vector<ConfigObject> objects;

      ConfigFile->get ( "Class", objects );

      for ( ConfigObject & aclass : objects )
      {
        parse_graphical ( ConfigFile, aclass );
      }

      objects.clear();
      ConfigFile->get ( "Window", objects );

      for ( ConfigObject & awindow : objects )
      {
        parse_window ( ConfigFile, awindow );
      }
    }
    catch ( dunedaq::conffwk::Exception const & ex )
    {
      ERROR ( "Configuration database load did not succeed",
              dbe::config::errors::parse ( ex ).c_str() );
    }
  }
}

void dbe::ui::config::info::parse_graphical ( std::shared_ptr<Configuration> ConfigFile,
                                              ConfigObject & Object )
{
  GraphicalClass GraphicalClassObject;

  GraphicalClassObject.GraphicalUID = QString ( Object.UID().c_str() );
  dunedaq::conffwk::class_t cinfo = ConfigFile->get_class_info ( Object.class_name(), false );

  std::vector<dunedaq::conffwk::attribute_t> const & attributes = cinfo.p_attributes;
  std::vector<dunedaq::conffwk::relationship_t> const & relations = cinfo.p_relationships;

  for ( dunedaq::conffwk::attribute_t const & attr : attributes )
  {
    QStringList data = dbe::config::api::get::direct::attribute<QStringList> ( Object, attr );

    if ( attr.p_name == "Name" )
    {
      GraphicalClassObject.DatabaseClassName = data.at ( 0 );
    }
    else if ( attr.p_name == "Generic Pixmap File" )
    {
      GraphicalClassObject.GenericPixmapFile = data.at ( 0 );
    }
    else if ( attr.p_name == "Used Pixmap File" )
    {
      GraphicalClassObject.UsedPixmapFile = data.at ( 0 );
    }
    else if ( attr.p_name == "Icon Bitmap File" )
    {
      GraphicalClassObject.BitmapFile = data.at ( 0 );
    }
    else if ( attr.p_name == "Icon Mask Bitmap File" )
    {
      GraphicalClassObject.BitmapMaskFile = data.at ( 0 );
    }
    else if ( attr.p_name == "Show All Attributes" )
    {
      GraphicalClassObject.ShowAllAttributes = convert::to<bool> (
                                                 data, attr.p_int_format );
    }
    else if ( attr.p_name == "Attributes" )
    {
      GraphicalClassObject.Attributes = data;
    }
    else if ( attr.p_name == "Show All Relationships" )
    {
      GraphicalClassObject.ShowAllRelationships = convert::to<bool> (
                                                    data, attr.p_int_format );
    }
    else if ( attr.p_name == "Relationships" )
    {
      GraphicalClassObject.Relationships = data;

    }
    else if ( attr.p_name == "Icon Title" )
    {
      GraphicalClassObject.IconTitle = data.at ( 0 );
    }
  }

  for ( dunedaq::conffwk::relationship_t const & relation : relations )
  {
    QStringList data;
    std::vector<ConfigObject> neighboring
    { };

    if ( dbe::config::api::info::relation::is_simple ( relation ) )
    {
      neighboring.push_back (
        dbe::config::api::graph::direct::linked<ConfigObject> ( Object, relation ) );
    }
    else
    {
      std::vector<ConfigObject> objects = dbe::config::api::graph::direct::linked <
                                          std::vector<ConfigObject >> ( Object, relation );
      neighboring.insert ( std::end ( neighboring ), std::begin ( objects ),
                           std::end ( objects ) );
    }

    for ( ConfigObject & RelObject : neighboring )
    {
      dunedaq::conffwk::class_t RelClassInfo = ConfigFile->get_class_info ( RelObject.class_name(),
                                                                       false );
      std::vector<dunedaq::conffwk::attribute_t> AttributeRelList = RelClassInfo.p_attributes;

      if ( RelObject.class_name() == "Dual Relationship" )
      {
        DualRelationship DRel;

        for ( dunedaq::conffwk::attribute_t & Attribute : AttributeRelList )
        {
          data = dbe::config::api::get::direct::attribute<QStringList> ( RelObject,
                                                                         Attribute );

          if ( Attribute.p_name == "Direct" )
          {
            DRel.Direct = data.at ( 0 );
          }
          else if ( Attribute.p_name == "Reverse" )
          {
            DRel.Reverse = data.at ( 0 );
          }
        }

        GraphicalClassObject.DualRelationships.push_back ( DRel );
      }
      else if ( RelObject.class_name() == "Init Attribute Value" )
      {
        InitAttributeFromEnv IRel;

        for ( dunedaq::conffwk::attribute_t & Attribute : AttributeRelList )
        {
          data = dbe::config::api::get::direct::attribute<QStringList> ( RelObject,
                                                                         Attribute );

          if ( Attribute.p_name == "Attribute Name" )
          {
            IRel.AttributeName = data.at ( 0 );
          }
          else if ( Attribute.p_name == "Environment Variables" )
          {
            IRel.EnvNames = data;
          }
        }

        GraphicalClassObject.NeedToInitialize.push_back ( IRel );
      }
    }
  }

  this_graphical.emplace ( GraphicalClassObject.GraphicalUID.toStdString(),
                           GraphicalClassObject );

}

void dbe::ui::config::info::parse_window ( std::shared_ptr<Configuration> database,
                                           ConfigObject & object )
{
  Window WindowObject;
  dunedaq::conffwk::class_t ClassInfo = database->get_class_info ( object.class_name(), false );
  std::vector<dunedaq::conffwk::attribute_t> const & attributes = ClassInfo.p_attributes;
  std::vector<dunedaq::conffwk::relationship_t> const & relations = ClassInfo.p_relationships;

  for ( dunedaq::conffwk::attribute_t const & attribute : attributes )
  {
    if ( attribute.p_name == "Title" )
    {
      WindowObject.Title = dbe::config::api::get::direct::attribute<QStringList> ( object,
                                                                                   attribute ).at (
                             0 );
    }
  }

  for ( dunedaq::conffwk::relationship_t const & relation : relations )
  {
    std::vector<ConfigObject> linkedobjects;

    if ( dbe::config::api::info::relation::is_simple ( relation ) )
    {
      linkedobjects.push_back (
        dbe::config::api::graph::direct::linked<ConfigObject> ( object, relation ) );
    }
    else
    {
      std::vector<ConfigObject> voisins = dbe::config::api::graph::direct::linked <
                                          std::vector<ConfigObject >> ( object, relation );

      if ( not voisins.empty() )
      {
        linkedobjects.insert ( std::end ( linkedobjects ), std::begin ( voisins ),
                               std::end ( voisins ) );
      }
    }

    for ( ConfigObject & neighbor : linkedobjects )
    {
      dunedaq::conffwk::class_t classinfo = database->get_class_info ( neighbor.class_name(),
                                                                  false );
      std::vector<dunedaq::conffwk::attribute_t> const & AttributeRelList = classinfo
                                                                       .p_attributes;
      std::vector<dunedaq::conffwk::relationship_t> const & RelationshipRelList = classinfo
                                                                             .p_relationships;

      if ( neighbor.class_name() != "Window Separator" )
      {
        for ( dunedaq::conffwk::attribute_t const & Attribute : AttributeRelList )
        {
          if ( Attribute.p_name == "Shown with children" )
          {
            QStringList neighbor_names
            { dbe::config::api::get::direct::attribute<QStringList> ( neighbor, Attribute ) };

            if ( neighbor_names.at ( 0 ) != "none" )
            {
              WindowObject.ShowChildren = true;
            }
            else
            {
              WindowObject.ShowChildren = false;
            }
          }
        }

        for ( dunedaq::conffwk::relationship_t const & RelationshipRel : RelationshipRelList )
        {
          QStringList RelDataList;
          std::vector<ConfigObject> neighboring_nodes;

          if ( dbe::config::api::info::relation::is_simple ( RelationshipRel ) )
          {
            neighboring_nodes.push_back (
              dbe::config::api::graph::direct::linked<ConfigObject> ( neighbor,
                                                                      RelationshipRel ) );
          }
          else
          {

            std::vector<ConfigObject> neighbors = dbe::config::api::graph::direct::linked <
                                                  std::vector<ConfigObject >> ( neighbor, RelationshipRel );

            neighboring_nodes.insert ( std::end ( neighboring_nodes ), std::begin ( neighbors ),
                                       std::end ( neighbors ) );
          }

          for ( ConfigObject & RelDataObject : neighboring_nodes )
          {
            RelDataList.push_back ( QString ( RelDataObject.UID().c_str() ) );
          }

          WindowObject.GraphicalClassesList.push_back ( RelDataList.at ( 0 ) );
        }
      }
    }
  }

  this_windows.emplace ( WindowObject.Title.toStdString(), WindowObject );
}

std::vector<dbe::Window> dbe::ui::config::info::windows() const
{
  std::vector<dbe::Window> windows;

  for ( auto const & x : this_windows )
  {
    windows.push_back ( x.second );
  }

  return windows;
}

std::vector<dbe::ViewConfiguration> dbe::ui::config::info::views() const
{
  std::vector<dbe::ViewConfiguration> views;

  for ( auto const & x : this_views )
  {
    views.push_back ( x.second );
  }

  return views;
}

std::vector<dbe::GraphicalClass> dbe::ui::config::info::graphicals() const
{
  std::vector<dbe::GraphicalClass> the_graphicals;

  for ( auto const & x : this_graphical )
  {
    the_graphicals.push_back ( x.second );
  }

  return the_graphicals;
}

