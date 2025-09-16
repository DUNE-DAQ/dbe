cmake_minimum_required(VERSION 3.12)
project(dbe VERSION 1.5.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

add_compile_options(-Wno-deprecated)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
find_package(oks REQUIRED)
find_package(conffwk REQUIRED)
find_package(logging REQUIRED)

find_package( Qt5 REQUIRED COMPONENTS Widgets Gui Core PrintSupport Svg)

add_definitions(
   -DQT_NO_DEBUG
   )

#====================================================================================#
# COMMON LIBS                                                                        #
#====================================================================================#
set(dbe_common_qt_libs
        Qt5::PrintSupport
        Qt5::Widgets
        Qt5::Gui 
        Qt5::Core
        Qt5::Svg
#	qtutils::qtutils
)

set(dbe_common_boost_libs
        Boost::program_options 
        )
        
set(dbe_common_test_libs
        Boost::system
        Boost::unit_test_framework
        )

set(dbe_common_tdaq_libs
        conffwk::conffwk
        ers::ers
        )
        
set(dbe_common_libs
        ${dbe_common_tdaq_libs}
        ${dbe_common_qt_libs}
        )        

include_directories(${CMAKE_CURRENT_BINARY_DIR})  # Needed for the automatically-generate user interface (UI) headers

#====================================================================================#
#                        LIB_DBE_CONFIG_API SOURCE FILES                             #
#====================================================================================#
set(dbe_config_api_src
                    internal/Command.cpp
                    internal/Conversion.cpp
                    internal/config_api.cpp
                    internal/config_api_set.cpp
                    internal/config_api_get.cpp
                    internal/config_api_info.cpp
                    internal/config_api_graph.cpp
                    internal/config_api_commands.cpp
   )


#====================================================================================#
#                        LIBDBE_STRUCTURE SOURCE FILES                               #
#====================================================================================#

qt5_wrap_cpp(dbe_structure_moc_src
                    include/dbe/CustomDelegate.hpp
                    include/dbe/CustomFileView.hpp
                    include/dbe/CustomTableView.hpp
                    include/dbe/CustomTreeView.hpp
                    include/dbe/FileModel.hpp
                    include/dbe/subtreeproxy.hpp
                    include/dbe/table.hpp
                    include/dbe/tableselection.hpp
                    include/dbe/tree.hpp
                    include/dbe/treeselection.hpp
)

set(dbe_structure_src
                    structure/CustomDelegate.cpp
                    structure/CustomFileView.cpp
                    structure/CustomTableView.cpp
                    structure/CustomTreeView.cpp
                    structure/FileModel.cpp
                    structure/treenode.cpp
                    structure/subtreeproxy.cpp
                    structure/table.cpp
                    structure/TableNode.cpp
                    structure/tableselection.cpp
                    structure/tree.cpp
                    structure/treeselection.cpp
                    structure/model_common_interface.cpp
  )

#====================================================================================#
#                           LIBDBE_WRAP SOURCE FILES                                 #
#====================================================================================#
set(dbe_wrap_src
                    internal/rwdacc.cpp
                    internal/dbcontroller.cpp
                    internal/dbaccessor.cpp
  )

#====================================================================================#
#                 SOURCE FILES FROM THE ATLAS TDAQ qtutils PACKAGE                   #
#====================================================================================#

qt5_wrap_cpp(dbe_qtutils_moc_src
    include/dbe/highlighter.hpp
    include/dbe/FilterTextEdit.hpp 
    include/dbe/qdebugstream.hpp
)

set(dbe_qtutils_src
	qtutils/highlighter.cpp
	qtutils/qdebugstream.cpp
)
	

#====================================================================================#
#                         LIBDBE_INTERNAL SOURCE FILES                               #
#====================================================================================#
qt5_wrap_cpp(dbe_internal_moc_src
                    include/dbe/Validator.hpp
                    include/dbe/messenger_proxy.hpp
                    include/dbe/datahandler.hpp                    
                    include/dbe/confaccessor.hpp
)

set(dbe_internal_src
                    internal/datahandler.cpp
                    internal/confaccessor.cpp
                    internal/messenger.cpp
                    internal/messenger_proxy.cpp
                    internal/config_ui_info.cpp
  )

#====================================================================================#
#                           LIBDBECORE SOURCE FILES                                  #
#====================================================================================#
set(dbe_core_ui
                    ui/BatchChangeWidget.ui
                    ui/CommitDialog.ui
                    ui/CreateDatabaseWidget.ui
                    ui/DBE.ui
                    ui/EditCombo.ui
                    ui/GraphView.ui
                    ui/IncludeFileWidget.ui
                    ui/NumericAttributeWidgetForm.ui
                    ui/ObjectCreator.ui
                    ui/ObjectEditor.ui
                    ui/OracleWidget.ui
                    ui/RelationshipWidgetForm.ui
                    ui/StringAttributeWidgetForm.ui
                    )
                    
qt5_wrap_ui(dbe_core_ui_header ${dbe_core_ui})

qt5_wrap_cpp(dbe_core_moc_src
                    include/dbe/TableTab.hpp
                    include/dbe/MyApplication.hpp
                    include/dbe/GraphicalClass.hpp
                    include/dbe/GraphView.hpp
                    include/dbe/MainWindow.hpp
                    include/dbe/CustomLineEdit.hpp
                    include/dbe/string_attr_text_edit.hpp
                    include/dbe/SearchComboBox.hpp
                    include/dbe/ValidatorComboBox.hpp
                    include/dbe/BatchChangeWidget.hpp
                    include/dbe/BuildingBlockEditors.hpp
                    include/dbe/CommitDialog.hpp
                    include/dbe/CreateDatabaseWidget.hpp
                    include/dbe/IncludeFileWidget.hpp
                    include/dbe/ObjectCreator.hpp
                    include/dbe/ObjectEditor.hpp
                    include/dbe/OracleWidget.hpp
)

set(dbe_core_src
                    internal/MainWindow.cpp
                    internal/StyleUtility.cpp
                    internal/Validator.cpp
                    object/CustomLineEdit.cpp
                    object/string_attr_text_edit.cpp
                    object/SearchComboBox.cpp
                    object/ValidatorComboBox.cpp
                    widgets/BatchChangeWidget.cpp
                    widgets/BuildingBlockEditors.cpp
                    widgets/CommitDialog.cpp
                    widgets/CreateDatabaseWidget.cpp
                    widgets/IncludeFileWidget.cpp
                    widgets/ObjectCreator.cpp
                    widgets/ObjectEditor.cpp
                    widgets/OracleWidget.cpp
                    graphical/GraphicalClass.cpp
                    graphical/GraphView.cpp
                    MyApplication.cpp
                    TableTab.cpp
  )

qt5_add_resources(qrc_resource icons/resource.qrc OPTIONS -name DBEimg)

daq_add_library(${dbe_core_src} 
         	${dbe_core_moc_src} 
		${dbe_core_ui_header} 
		${qrc_resource} 
		${dbe_structure_src} 
		${dbe_structure_moc_src} 
		${dbe_internal_src} 
		${dbe_internal_moc_src} 
		${dbe_wrap_src} 
		${dbe_config_api_src}
		${qtutils_src}
		LINK_LIBRARIES
		${dbe_common_libs}
)


#====================================================================================#
# DATABASE EDITOR (DBE)                                                              #
#====================================================================================#

daq_add_application(dbe_main Main.cpp LINK_LIBRARIES dbe ${dbe_common_boost_libs})

#====================================================================================#
# DBE GRAPHTOOL (DBE_GTOOL)                                                          #
#====================================================================================#

set(dbe_gtool_src
      graphtool/gtool.cpp
      graphtool/segregate.cpp
      graphtool/stats.cpp
      )

daq_add_application(dbe_gtool graphtool.cpp ${dbe_gtool_src} LINK_LIBRARIES dbe ${dbe_common_boost_libs})

#====================================================================================#
#                            SCHEMA EDITOR SOURCE FILES, ETC.                        #
#====================================================================================#
set(schema_core_ui
                    ui/SchemaAttributeEditor.ui
                    ui/SchemaClassEditor.ui
                    ui/SchemaMainWindow.ui
                    ui/SchemaMethodEditor.ui
                    ui/SchemaMethodImplementationEditor.ui
                    ui/SchemaNoteEditor.ui
                    ui/SchemaRelationshipEditor.ui
                    ui/SchemaFileInfo.ui
                    ui/SchemaSettings.ui
                    )

qt5_wrap_ui(schema_core_ui_header ${schema_core_ui})

set(schema_core_src
                    SchemaEditor/SchemaAttributeEditor.cpp
                    SchemaEditor/SchemaCustomMethodImplementationModel.cpp
                    SchemaEditor/SchemaGraphicArrow.cpp
                    SchemaEditor/SchemaGraphicSegmentedArrow.cpp
                    SchemaEditor/SchemaMainWindow.cpp
                    SchemaEditor/SchemaClassEditor.cpp
                    SchemaEditor/SchemaCustomMethodModel.cpp
                    SchemaEditor/SchemaGraphicObject.cpp
                    SchemaEditor/SchemaGraphicNote.cpp
                    SchemaEditor/SchemaNoteEditor.cpp
                    SchemaEditor/SchemaMethodEditor.cpp
                    SchemaEditor/SchemaCommand.cpp
                    SchemaEditor/SchemaCustomModelInterface.cpp
                    SchemaEditor/SchemaGraphicsScene.cpp
                    SchemaEditor/SchemaMethodImplementationEditor.cpp
                    SchemaEditor/SchemaCustomAttributeModel.cpp
                    SchemaEditor/SchemaCustomRelationshipModel.cpp
                    SchemaEditor/SchemaKernelWrapper.cpp
                    SchemaEditor/SchemaRelationshipEditor.cpp
                    SchemaEditor/SchemaCustomFileModel.cpp
                    SchemaEditor/SchemaCustomTableModel.cpp
                    SchemaEditor/SchemaCustomSuperClassModel.cpp
                    SchemaEditor/SchemaCustomSubClassModel.cpp
                    SchemaEditor/SchemaTab.cpp
                    SchemaEditor/SchemaFileInfo.cpp
                    SchemaEditor/SchemaStyle.cpp
                    SchemaEditor/SchemaSettings.cpp
  )

qt5_wrap_cpp(schema_core_moc_src
                    include/dbe/SchemaAttributeEditor.hpp
                    include/dbe/SchemaClassEditor.hpp
                    include/dbe/SchemaCustomFileModel.hpp
                    include/dbe/SchemaCustomModelInterface.hpp
                    include/dbe/SchemaCustomTableModel.hpp
                    include/dbe/SchemaGraphicObject.hpp
                    include/dbe/SchemaGraphicNote.hpp
                    include/dbe/SchemaGraphicsScene.hpp
                    include/dbe/SchemaKernelWrapper.hpp
                    include/dbe/SchemaMainWindow.hpp
                    include/dbe/SchemaMethodEditor.hpp
                    include/dbe/SchemaMethodImplementationEditor.hpp
                    include/dbe/SchemaNoteEditor.hpp
                    include/dbe/SchemaRelationshipEditor.hpp
                    include/dbe/SchemaTab.hpp
                    include/dbe/SchemaFileInfo.hpp
                    include/dbe/SchemaSettings.hpp
)

qt5_add_resources(qrc_SchemaResource icons/SchemaResource.qrc OPTIONS -name SchemaResource)

daq_add_application(schemaeditor SchemaMain.cpp ${schema_core_src} ${schema_core_moc_src} ${schema_core_ui_header} ${qrc_SchemaResource} LINK_LIBRARIES dbe ${dbe_common_libs} oks::oks ${dbe_common_boost_libs} )

daq_add_unit_test(confaccessor_test LINK_LIBRARIES dbe)
daq_add_unit_test(config_api_set_test LINK_LIBRARIES dbe)
daq_add_unit_test(config_api_info_test LINK_LIBRARIES dbe)

daq_install()
