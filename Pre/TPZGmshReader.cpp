//
//  TPZGmshReader.cpp
//  PZ
//
//  Created by Omar on 2/7/16.
//
//

#include "TPZGmshReader.h"

#include "pzgeopoint.h"
#include "TPZGeoLinear.h"
#include "TPZGeoCube.h"
#include "pzgeotetrahedra.h"
#include "tpzcube.h"
#include "pzgeopyramid.h"


#include "tpzquadraticline.h"
#include "tpzquadratictrig.h"
#include "tpzquadraticquad.h"
#include "tpzquadraticcube.h"
#include "tpzquadratictetra.h"
#include "tpzquadraticprism.h"
#include "tpzgeoblend.h"
#include "tpzpoint.h"
#include "pzrefpoint.h"

#include "pzgeoelside.h"
#include "tpzgeoblend.h"
#include <tpzarc3d.h>

#include "TPZRefPattern.h"
#include "tpzgeoelrefpattern.h"
#include "TPZGeoElement.h"


TPZGmshReader::TPZGmshReader() {
    
    m_format_version = "4.0";
    m_n_volumes = 0;
    m_n_surfaces = 0;
    m_n_curves = 0;
    m_n_points = 0;
    m_n_physical_volumes = 0;
    m_n_physical_surfaces = 0;
    m_n_physical_curves = 0;
    m_n_physical_points = 0;
    m_dimension = 0;
    m_characteristic_lentgh = 1.0;
    m_dim_entity_tag_and_physical_tag.Resize(4);
    m_dim_physical_tag_and_name.Resize(4);
    m_dim_name_and_physical_tag.Resize(4);
    m_dim_physical_tag_and_physical_tag.Resize(4);

    m_entity_index.Resize(0);
    
}//method

TPZGmshReader::~TPZGmshReader() {
    
}//method

TPZGeoMesh * TPZGmshReader::GeometricGmshMesh(std::string file_name, TPZGeoMesh *gmesh_input){
    
    if (m_format_version == "3.0" || m_format_version == "3") {
        return GeometricGmshMesh3(file_name,gmesh_input);
    }else if(m_format_version == "4.1"){
        return GeometricGmshMesh4(file_name,gmesh_input);
    }
    std::cout << "TPZGmshReader:: Lastest version supported 4.1 " << std::endl;
    std::cout << "TPZGmshReader:: This reader supports only the lastest version " << std::endl;
    std::cout << "TPZGmshReader:: Reader no available for the msh file version = " << m_format_version << std::endl;
    DebugStop();
    return NULL;
}

void TPZGmshReader::PrintPartitionSummary(std::ostream & out){
    
    out << std::endl;
    out << "TPZGmshReader geometrical partition summary " << std::endl;
    out << "File format         = " << m_format_version << std::endl;
    out << "Geometry dimension  = " << m_dimension << std::endl;
    out << "Number of volumes   = " << m_n_volumes << std::endl;
    out << "Number of surfaces  = " << m_n_surfaces << std::endl;
    out << "Number of curves    = " << m_n_curves << std::endl;
    out << "Number of points    = " << m_n_points << std::endl;
    out << "Number of volumes with physical tag     = " << m_n_physical_volumes << std::endl;
    out << "Number of surfaces with physical tag    = " << m_n_physical_surfaces << std::endl;
    out << "Number of curves with physical tag      = " << m_n_physical_curves << std::endl;
    out << "Number of points with physical tag      = " << m_n_physical_points << std::endl;
    out << "Number of elements by type : " << std::endl;
    out << "Points          : " << m_n_point_els << std::endl;
    out << "Lines           : " << m_n_line_els << std::endl;
    out << "Triangles       : " << m_n_triangle_els << std::endl;
    out << "Quadrilaterals  : " << m_n_quadrilateral_els << std::endl;
    out << "Tetrahera       : " << m_n_tetrahedron_els << std::endl;
    out << "Hexahedra       : " << m_n_hexahedron_els << std::endl;
    out << "Prism           : " << m_n_prism_els << std::endl;
    out << "Pyramids        : " << m_n_pyramid_els << std::endl;
    int n_vols_els = m_n_pyramid_els + m_n_prism_els + m_n_hexahedron_els + m_n_tetrahedron_els;
    int n_surf_els = m_n_triangle_els + m_n_quadrilateral_els;
    out << "Number of elements by dimension : " << std::endl;
    out << "3D elements : " << n_vols_els << std::endl;
    out << "2D elements : " << n_surf_els << std::endl;
    out << "1D elements : " << m_n_line_els << std::endl;
    out << "0D elements : " << m_n_point_els << std::endl;
    out << "Characteristic length = " << m_characteristic_lentgh <<std::endl;
    out << std::endl;

}
/// Copy constructor
TPZGmshReader::TPZGmshReader(const TPZGmshReader & other){
    m_format_version                = other.m_format_version;
    m_n_volumes                     = other.m_n_volumes;
    m_n_surfaces                    = other.m_n_surfaces;
    m_n_curves                      = other.m_n_curves;
    m_n_points                      = other.m_n_points;
    m_n_physical_volumes            = other.m_n_physical_volumes;
    m_n_physical_surfaces           = other.m_n_physical_surfaces;
    m_n_physical_curves             = other.m_n_physical_curves;
    m_n_physical_points             = other.m_n_physical_points;
    m_dimension                     = other.m_dimension;
    m_characteristic_lentgh         = other.m_characteristic_lentgh;
    m_dim_entity_tag_and_physical_tag   = other.m_dim_entity_tag_and_physical_tag;
    m_dim_physical_tag_and_name         = other.m_dim_physical_tag_and_name;
    m_dim_name_and_physical_tag         = other.m_dim_name_and_physical_tag;
    m_dim_physical_tag_and_physical_tag = other.m_dim_physical_tag_and_physical_tag;
    m_entity_index                      = other.m_entity_index;
}

/// Assignement constructo{
const TPZGmshReader & TPZGmshReader::operator=(const TPZGmshReader & other){
    /// check for self-assignment
    if(&other == this){
        return *this;
    }
    m_format_version                = other.m_format_version;
    m_n_volumes                     = other.m_n_volumes;
    m_n_surfaces                    = other.m_n_surfaces;
    m_n_curves                      = other.m_n_curves;
    m_n_points                      = other.m_n_points;
    m_n_physical_volumes            = other.m_n_physical_volumes;
    m_n_physical_surfaces           = other.m_n_physical_surfaces;
    m_n_physical_curves             = other.m_n_physical_curves;
    m_n_physical_points             = other.m_n_physical_points;
    m_dimension                     = other.m_dimension;
    m_characteristic_lentgh         = other.m_characteristic_lentgh;
    m_dim_entity_tag_and_physical_tag   = other.m_dim_entity_tag_and_physical_tag;
    m_dim_physical_tag_and_name         = other.m_dim_physical_tag_and_name;
    m_dim_name_and_physical_tag         = other.m_dim_name_and_physical_tag;
    m_dim_physical_tag_and_physical_tag = other.m_dim_physical_tag_and_physical_tag;
    m_entity_index                      = other.m_entity_index;
    return *this;
}

TPZGeoMesh * TPZGmshReader::GeometricGmshMesh4(std::string file_name, TPZGeoMesh *gmesh_input){
    
    //  Mesh Creation
    TPZGeoMesh * gmesh = gmesh_input;
    if(!gmesh) gmesh = new TPZGeoMesh;
    
    {
        
        // reading a general mesh information by filter
        std::ifstream read (file_name.c_str());
        if(!read)
        {
            std::cout << "Couldn't open the file " << file_name << std::endl;
            DebugStop();
        }
        
        if (!read) {
            std::cout << "Gmsh Reader: the mesh file path is wrong " << std::endl;
            DebugStop();
        }
        
        while(read)
        {
            char buf[1024];
            read.getline(buf, 1024);
            std::string str(buf);
            
            if(str == "$MeshFormat" || str == "$MeshFormat\r")
            {
                read.getline(buf, 1024);
                std::string str(buf);
                std::cout << "Reading mesh format = " << str << std::endl;
                
            }
            
            if(str == "$PhysicalNames" || str == "$PhysicalNames\r" )
            {
                
                int64_t n_physical_names;
                read >> n_physical_names;
                int max_dimension = 0;
                
                int dimension, id;
                std::string name;
                std::pair<int, std::string> chunk;
                
                for (int64_t i_name = 0; i_name < n_physical_names; i_name++) {
                    
                    read.getline(buf, 1024);
                    read >> dimension;
                    read >> id;
                    read >> name;
                    name.erase(0,1);
                    name.erase(name.end()-1,name.end());
                    m_dim_physical_tag_and_name[dimension][id] = name;
                    
                    if(m_dim_name_and_physical_tag[dimension].find(name) == m_dim_name_and_physical_tag[dimension].end())
                    {
                        std::cout << "Automatically associating " << name << " with material id " << id << std::endl;
                        m_dim_name_and_physical_tag[dimension][name] = id;
                    }
                    else
                    {
                        int pzmatid = m_dim_name_and_physical_tag[dimension][name];
                        std::cout << "Associating " << name << " with material id " << id <<
                        " with pz material id " << pzmatid << std::endl;
                    }
                    
                    m_dim_physical_tag_and_physical_tag[dimension][id] = m_dim_name_and_physical_tag[dimension][name];
                    
                    if (max_dimension < dimension) {
                        max_dimension = dimension;
                    }
                }
                m_dimension = max_dimension; //  for coherence
                gmesh->SetDimension(max_dimension);
                
                char buf_end[1024];
                read.getline(buf_end, 1024);
                read.getline(buf_end, 1024);
                std::string str_end(buf_end);
                if(str_end == "$EndPhysicalNames" || str_end == "$EndPhysicalNames\r")
                {
                    std::cout << "Read mesh physical entities = " << n_physical_names << std::endl;
                }
                continue;
            }
            
            if(str == "$Entities" || str == "$Entities\r")
            {
                read >> m_n_points;
                read >> m_n_curves;
                read >> m_n_surfaces;
                read >> m_n_volumes;
                
                
                int n_physical_tag;
                std::pair<int, std::vector<int> > chunk;
                /// Entity bounding box data
                REAL x_min, y_min, z_min;
                REAL x_max, y_max, z_max;
                std::vector<int> n_entities = {m_n_points,m_n_curves,m_n_surfaces,m_n_volumes};
                std::vector<int> n_entities_with_physical_tag = {0,0,0,0};
                
                
                for (int i_dim = 0; i_dim <4; i_dim++) {
                    for (int64_t i_entity = 0; i_entity < n_entities[i_dim]; i_entity++) {
                        
                        read.getline(buf, 1024);
                        read >> chunk.first;
                        read >> x_min;
                        read >> y_min;
                        read >> z_min;
                        if(i_dim > 0)
                        {
                            read >> x_max;
                            read >> y_max;
                            read >> z_max;
                        }
                        read >> n_physical_tag;
                        chunk.second.resize(n_physical_tag);
                        for (int i_data = 0; i_data < n_physical_tag; i_data++) {
                            read >> chunk.second[i_data];
                        }
                        if(i_dim > 0)
                        {
                            size_t n_bounding_points;
                            read >> n_bounding_points;
                            for (int i_data = 0; i_data < n_bounding_points; i_data++) {
                                int point_tag;
                                read >> point_tag;
                            }
                        }
                        n_entities_with_physical_tag[i_dim] += n_physical_tag;
                        m_dim_entity_tag_and_physical_tag[i_dim].insert(chunk);
                    }
                }

                m_n_physical_points = n_entities_with_physical_tag[0];
                m_n_physical_curves = n_entities_with_physical_tag[1];
                m_n_physical_surfaces = n_entities_with_physical_tag[2];
                m_n_physical_volumes = n_entities_with_physical_tag[3];
                
                char buf_end[1024];
                read.getline(buf_end, 1024);
                read.getline(buf_end, 1024);
                std::string str_end(buf_end);
                if(str_end == "$EndEntities" || str_end == "$EndEntities\r")
                {
                    std::cout << "Read mesh entities = " <<  m_n_points + m_n_curves + m_n_surfaces + m_n_volumes << std::endl;
                    std::cout << "Read mesh entities with physical tags = " <<  m_n_physical_points + m_n_physical_curves + m_n_physical_surfaces + m_n_physical_volumes << std::endl;
                }
                continue;
            }
            
            if(str == "$Nodes" || str == "$Nodes\r")
            {
                
                int64_t n_entity_blocks, n_nodes, min_node_tag, max_node_tag;
                read >> n_entity_blocks;
                read >> n_nodes;
                read >> min_node_tag;
                read >> max_node_tag;
                
                int64_t node_id;
                double nodecoordX , nodecoordY , nodecoordZ ;
                gmesh -> NodeVec().Resize(max_node_tag);
                gmesh->SetMaxNodeId(max_node_tag);
                // needed for node insertion
                const int64_t Tnodes = max_node_tag;
                TPZVec <TPZGeoNode> Node(Tnodes);
                
                int entity_tag, entity_dim, entity_parametric, entity_nodes;
                for (int64_t i_block = 0; i_block < n_entity_blocks; i_block++)
                {
                    read.getline(buf, 1024);
                    read >> entity_dim;
                    read >> entity_tag;
                    read >> entity_parametric;
                    read >> entity_nodes;
                    
                    if (entity_parametric != 0) {
                        std::cout << "TPZGmshReader:: Characteristic not implemented." << std::endl;
                        DebugStop();
                    }
                    
                    TPZManVector<int64_t,10> nodeids(entity_nodes,-1);
                    for (int64_t inode = 0; inode < entity_nodes; inode++) {
                        read >> nodeids[inode];
                    }
                    for (int64_t inode = 0; inode < entity_nodes; inode++) {
                        read >> nodecoordX;
                        read >> nodecoordY;
                        read >> nodecoordZ;
                        
                        Node[nodeids[inode]-1].SetNodeId(nodeids[inode]-1);
                        Node[nodeids[inode]-1].SetCoord(0,nodecoordX/m_characteristic_lentgh);
                        Node[nodeids[inode]-1].SetCoord(1,nodecoordY/m_characteristic_lentgh);
                        Node[nodeids[inode]-1].SetCoord(2,nodecoordZ/m_characteristic_lentgh);
                        gmesh->NodeVec()[nodeids[inode]-1] = Node[nodeids[inode]-1];
                        
                    }
                }
                
                char buf_end[1024];
                read.getline(buf_end, 1024);
                read.getline(buf_end, 1024);
                std::string str_end(buf_end);
                if(str_end == "$EndNodes" || str_end == "$EndNodes\r")
                {
                    std::cout << "Read mesh nodes = " <<  gmesh->NNodes() << std::endl;
                }
                continue;
            }
            
            if(str == "$Elements" || str == "$Elements\r")
            {
                
                int64_t n_entity_blocks, n_elements, min_element_tag, max_element_tag;
                read >> n_entity_blocks;
                read >> n_elements;
                read >> min_element_tag;
                read >> max_element_tag;
                gmesh->SetMaxElementId(n_elements-1);
                
                int entity_tag, entity_dim, entity_el_type, entity_elements;
                for (int64_t i_block = 0; i_block < n_entity_blocks; i_block++)
                {
                    read.getline(buf, 1024);
                    read >> entity_dim;
                    read >> entity_tag;
                    read >> entity_el_type;
                    read >> entity_elements;
                    
                    if(entity_elements == 0){
                        std::cout << "The entity with tag " << entity_tag << " does not have elements to insert" << std::endl;
                    }
                    
                    for (int64_t iel = 0; iel < entity_elements; iel++) {
                        int physical_identifier;
                        int n_physical_identifier = 0;
                        if(m_dim_entity_tag_and_physical_tag[entity_dim].find(entity_tag) != m_dim_entity_tag_and_physical_tag[entity_dim].end())
                        {
                            n_physical_identifier = m_dim_entity_tag_and_physical_tag[entity_dim][entity_tag].size();
                        }
                        bool physical_identifier_Q = n_physical_identifier != 0;
                        if(physical_identifier_Q)
                        {
                            int gmsh_physical_identifier = m_dim_entity_tag_and_physical_tag[entity_dim][entity_tag][0];
                            physical_identifier = m_dim_physical_tag_and_physical_tag[entity_dim][gmsh_physical_identifier];
                            if(n_physical_identifier !=1){
                                std::cout << "The entity with tag " << entity_tag << std::endl;
                                std::cout << "Has associated the following physical tags : " << std::endl;
                                for (int i_data = 0; i_data < n_physical_identifier; i_data++) {
                                    std::cout << m_dim_entity_tag_and_physical_tag[entity_dim][entity_tag][i_data] << std::endl;
                                }
                                
                                std::cout << "Automatically, the assgined pz physical tag = " << physical_identifier << " is used.  The other ones are dropped out." << std::endl;
                            }
                            
                            
                            read.getline(buf, 1024);
                            int el_identifier, n_el_nodes;
                            n_el_nodes = GetNumberofNodes(entity_el_type);
                            read >> el_identifier;
                            std::vector<int> node_identifiers(n_el_nodes);
                            for (int i_node = 0; i_node < n_el_nodes; i_node++) {
                                read >> node_identifiers[i_node];
                            }
                            /// Internally the nodes index and element index is converted to zero based indexation
                            InsertElement(gmesh, physical_identifier, entity_el_type, el_identifier, node_identifiers);
                            
                        }else{
                            read.getline(buf, 1024);
                            int el_identifier, n_el_nodes;
                            n_el_nodes = GetNumberofNodes(entity_el_type);
                            read >> el_identifier;
                            std::vector<int> node_identifiers(n_el_nodes);
                            for (int i_node = 0; i_node < n_el_nodes; i_node++) {
                                read >> node_identifiers[i_node];
                            }
                            std::cout << "The entity with tag " << entity_tag << " does not have a physical tag, element " << el_identifier << " skipped " << std::endl;
                        }

                    }
                }
                
                char buf_end[1024];
                read.getline(buf_end, 1024);
                read.getline(buf_end, 1024);
                std::string str_end(buf_end);
                if(str_end == "$EndElements" || str_end == "$EndElements\r")
                {
                    std::cout << "Read mesh elements = " << gmesh->NElements() << std::endl;
                }
                continue;
            }
            
        }
        
    }
    
    std::cout << "Read General Mesh Data -> done!" << std::endl;
    std::cout << "Number of elements " << gmesh->NElements() << std::endl;
    gmesh->BuildConnectivity();
    std::cout << "Geometric Mesh Connectivity -> done!" << std::endl;
    return gmesh;
    
}

void TPZGmshReader::InsertElement(TPZGeoMesh * gmesh, int & physical_identifier, int & el_type, int & el_identifier, std::vector<int> & node_identifiers){
    
    TPZManVector <int64_t,15> Topology;
    int n_nodes = node_identifiers.size();
    Topology.Resize(n_nodes, 0);
    for (int k_node = 0; k_node<n_nodes; k_node++) {
        Topology[k_node] = node_identifiers[k_node]-1;
    }
    
    el_identifier--;
    switch (el_type) {
        case 1:
        {   // Line
            new TPZGeoElRefPattern< pzgeom::TPZGeoLinear> (el_identifier, Topology, physical_identifier, *gmesh);
            m_n_line_els++;
        }
            break;
        case 2:
        {
            // Triangle
            new TPZGeoElRefPattern< pzgeom::TPZGeoTriangle> (el_identifier, Topology, physical_identifier, *gmesh);
            m_n_triangle_els++;
            
        }
            break;
        case 3:
        {
            // Quadrilateral
            new TPZGeoElRefPattern< pzgeom::TPZGeoQuad> (el_identifier, Topology, physical_identifier, *gmesh);
            m_n_quadrilateral_els++;
            
        }
            break;
        case 4:
        {
            // Tetrahedron
            new TPZGeoElRefPattern< pzgeom::TPZGeoTetrahedra> (el_identifier, Topology, physical_identifier, *gmesh);
            m_n_tetrahedron_els++;
            
        }
            break;
        case 5:
        {
            // Hexahedra
            new TPZGeoElRefPattern< pzgeom::TPZGeoCube> (el_identifier, Topology, physical_identifier, *gmesh);
            m_n_hexahedron_els++;
        }
            break;
        case 6:
        {
            // Prism
            new TPZGeoElRefPattern< pzgeom::TPZGeoPrism> (el_identifier, Topology, physical_identifier, *gmesh);
            m_n_prism_els++;
        }
            break;
        case 7:
        {
            // Pyramid
            new TPZGeoElRefPattern< pzgeom::TPZGeoPyramid> (el_identifier, Topology, physical_identifier, *gmesh);
            m_n_pyramid_els++;
        }
            break;
        case 8:
        {
            // Quadratic Line
            new TPZGeoElRefPattern< pzgeom::TPZQuadraticLine> (el_identifier, Topology, physical_identifier, *gmesh);
            m_n_line_els++;
        }
            break;
        case 9:
        {
            // Triangle
            new TPZGeoElRefPattern< pzgeom::TPZQuadraticTrig> (el_identifier, Topology, physical_identifier, *gmesh);
            m_n_triangle_els++;
        }
            break;
        case 10:
        {
            TPZManVector <int64_t,15> Topology_c(n_nodes-1);
            for (int k_node = 0; k_node < n_nodes-1; k_node++) { /// Gmsh representation Quadrangle8 and Quadrangle9, but by default Quadrangle9 is always generated. (?_?).
                Topology_c[k_node] = Topology[k_node];
            }
            // Quadrilateral
            new TPZGeoElRefPattern< pzgeom::TPZQuadraticQuad> (el_identifier, Topology_c, physical_identifier, *gmesh);
            m_n_quadrilateral_els++;
        }
            break;
        case 11:
        {
            // Tetrahedron
            new TPZGeoElRefPattern< pzgeom::TPZQuadraticTetra> (el_identifier, Topology, physical_identifier, *gmesh);
            m_n_tetrahedron_els++;
            
        }
            break;
        case 12:
        {
            // Hexahedra
            new TPZGeoElRefPattern< pzgeom::TPZQuadraticCube> (el_identifier, Topology, physical_identifier, *gmesh);
            m_n_hexahedron_els++;
        }
            break;
        case 13:
        {
            // Prism
            new TPZGeoElRefPattern< pzgeom::TPZQuadraticPrism> (el_identifier, Topology, physical_identifier, *gmesh);
            m_n_pyramid_els++;
        }
            break;
        case 15:{
            // Point
            new TPZGeoElement< pzgeom::TPZGeoPoint, pzrefine::TPZRefPoint> (el_identifier, Topology, physical_identifier, *gmesh);
            m_n_point_els++;
        }
            break;
        default:
        {
            std::cout << "Element not impelemented." << std::endl;
            DebugStop();
        }
            break;
    }
    
}

int TPZGmshReader::GetNumberofNodes(int & el_type){
    
    int n_nodes;
    switch (el_type) {
        case 1:
        {   // Line
            n_nodes = 2;
        }
            break;
        case 2:
        {
            // Triangle
            n_nodes = 3;
        }
            break;
        case 3:
        {
            // Quadrilateral
            n_nodes = 4;
        }
            break;
        case 4:
        {
            // Tetrahedron
            n_nodes = 4;
        }
            break;
        case 5:
        {
            // Hexahedra
            n_nodes = 8;
        }
            break;
        case 6:
        {
            // Prism
            n_nodes = 6;
        }
            break;
        case 7:
        {
            // Pyramid
            n_nodes = 5;
        }
            break;
        case 8:
        {
            // Quadratic Line
            n_nodes = 3;
        }
            break;
        case 9:
        {
            // Quadratic Triangle
            n_nodes = 6;
        }
            break;
        case 10:
        {
            // Quadratic Quadrilateral
            n_nodes = 9;
        }
            break;
        case 11:
        {
            // Quadratic Tetrahedron
            n_nodes = 10;
            
        }
            break;
        case 12:
        {
            // Quadratic Hexahedra
            n_nodes = 20;
        }
            break;
        case 13:
        {
            // Quadratic Prism
            n_nodes = 15;
        }
            break;
        case 15:{
            // Point
            n_nodes = 1;
        }
            break;
        default:
        {
            std::cout << "Element not impelemented." << std::endl;
            n_nodes = 0;
            DebugStop();
        }
            break;
    }
    
    return n_nodes;
}

TPZGeoMesh * TPZGmshReader::GeometricGmshMesh3(std::string file_name, TPZGeoMesh *gmesh_input)
{
    
    //  Mesh Creation
    TPZGeoMesh * gmesh = gmesh_input;
    if(!gmesh) gmesh = new TPZGeoMesh;
    int dimension = 0;
    gmesh->SetDimension(dimension);
    
    {
        
        // reading a general mesh information by filter
        std::ifstream read (file_name.c_str());
        if(!read)
        {
            std::cout << "Couldn't open the file " << file_name << std::endl;
            DebugStop();
        }
        
        if (!read) {
            std::cout << "Gmsh Reader: the mesh file path is wrong " << std::endl;
            DebugStop();
        }
        
        while(read)
        {
            char buf[1024];
            read.getline(buf, 1024);
            std::string str(buf);
            
            if(str == "$MeshFormat" || str == "$MeshFormat\r")
            {
                read.getline(buf, 1024);
                std::string str(buf);
                std::cout << "Reading mesh format = " << str << std::endl;
                
            }
            
            if(str == "$PhysicalNames" || str == "$PhysicalNames\r" )
            {
                
                int64_t n_entities;
                read >> n_entities;
                int max_dimension = 0;
                
                int dimension, id;
                std::string name;
                std::pair<int, std::string> chunk;
                
                for (int64_t inode = 0; inode < n_entities; inode++) {
                    
                    read.getline(buf, 1024);
                    read >> dimension;
                    read >> id;
                    read >> name;
                    name.erase(0,1);
                    name.erase(name.end()-1,name.end());
                    m_dim_physical_tag_and_name[dimension][id] = name;
                    
                    if(m_dim_name_and_physical_tag[dimension].find(name) == m_dim_name_and_physical_tag[dimension].end())
                    {
                        std::cout << "Automatically associating " << name << " with material id " << id << std::endl;
                        m_dim_name_and_physical_tag[dimension][name] = id;
                    }
                    else
                    {
                        int pzmatid = m_dim_name_and_physical_tag[dimension][name];
                        std::cout << "Associating " << name << " with material id " << id <<
                    " with pz material id " << pzmatid << std::endl;
                    }
                    
                    m_dim_physical_tag_and_physical_tag[dimension][id] = m_dim_name_and_physical_tag[dimension][name];
                    
                    if (max_dimension < dimension) {
                        max_dimension = dimension;
                    }
                }
                gmesh->SetDimension(max_dimension);
                
                char buf_end[1024];
                read.getline(buf_end, 1024);
                read.getline(buf_end, 1024);
                std::string str_end(buf_end);
                if(str_end == "$EndPhysicalNames" || str_end == "$EndPhysicalNames\r")
                {
                    std::cout << "Read mesh physical entities = " << n_entities << std::endl;
                }
                continue;
            }
            
            if(str == "$Nodes" || str == "$Nodes\r")
            {
                
                int64_t n_nodes;
                read >> n_nodes;
                
                int64_t node_id;
                double nodecoordX , nodecoordY , nodecoordZ ;
                gmesh -> NodeVec().Resize(n_nodes);
                gmesh->SetMaxNodeId(n_nodes-1);
                
                // needed for node insertion
                const int64_t Tnodes = n_nodes;
                TPZVec <TPZGeoNode> Node(Tnodes);
                
                for (int64_t inode = 0; inode < n_nodes; inode++) {
                    
                    read.getline(buf, 1024);
                    read >> node_id;
                    read >> nodecoordX;
                    read >> nodecoordY;
                    read >> nodecoordZ;
                    
                    Node[node_id-1].SetNodeId(node_id-1);
                    Node[node_id-1].SetCoord(0,nodecoordX/m_characteristic_lentgh);
                    Node[node_id-1].SetCoord(1,nodecoordY/m_characteristic_lentgh);
                    Node[node_id-1].SetCoord(2,nodecoordZ/m_characteristic_lentgh);
                    gmesh->NodeVec()[node_id-1] = Node[node_id-1];
                    
                }
                
                
                char buf_end[1024];
                read.getline(buf_end, 1024);
                read.getline(buf_end, 1024);
                std::string str_end(buf_end);
                if(str_end == "$EndNodes" || str_end == "$EndNodes\r")
                {
                    std::cout << "Read mesh nodes = " <<  gmesh->NNodes() << std::endl;
                }
                continue;
            }
            
            if(str == "$Elements" || str == "$Elements\r")
            {
                
                int64_t n_elements;
                read >> n_elements;
                gmesh->SetMaxElementId(n_elements-1);
                
                for (int64_t iel = 0; iel < n_elements; iel++) {
                    this->InsertElement(gmesh, read);
                }
                
                char buf_end[1024];
                read.getline(buf_end, 1024);
                read.getline(buf_end, 1024);
                std::string str_end(buf_end);
                if(str_end == "$EndElements" || str_end == "$EndElements\r")
                {
                    std::cout << "Read mesh elements = " << gmesh->NElements() << std::endl;
                }
                continue;
            }
            
        }
        
    }
    
    std::cout << "Read General Mesh Data -> done!" << std::endl;
    std::cout << "Number of elements " << gmesh->NElements() << std::endl;
    gmesh->BuildConnectivity();
    std::cout << "Geometric Mesh Connectivity -> done!" << std::endl;
    return gmesh;
    
}// End Method

void TPZGmshReader::SetCharacteristiclength(REAL length)
{
    m_characteristic_lentgh = length;
}

void TPZGmshReader::SetFormatVersion(std::string format_version)
{
    m_format_version = format_version;
}

/** @brief Insert elements following msh file format */
bool TPZGmshReader::InsertElement(TPZGeoMesh * gmesh, std::ifstream & line){
    
    // first implementation based on linear elements: http://gmsh.info/doc/texinfo/gmsh.html#File-formats
    TPZManVector <int64_t,1> TopolPoint(1);
    TPZManVector <int64_t,2> TopolLine(2);
    TPZManVector <int64_t,3> TopolTriangle(3);
    TPZManVector <int64_t,4> TopolQuad(4);
    TPZManVector <int64_t,4> TopolTet(4);
    TPZManVector <int64_t,5> TopolPyr(5);
    TPZManVector <int64_t,6> TopolPrism(6);
    TPZManVector <int64_t,8> TopolHex(8);
    
    TPZManVector <int64_t,2> TopolLineQ(3);
    TPZManVector <int64_t,6> TopolTriangleQ(6);
    TPZManVector <int64_t,8> TopolQuadQ(8);
    TPZManVector <int64_t,10> TopolTetQ(10);
    TPZManVector <int64_t,5> TopolPyrQ(14);
    TPZManVector <int64_t,15> TopolPrismQ(15);
    TPZManVector <int64_t,8> TopolHexQ(20);
    
    
    int64_t element_id, type_id, div_id, physical_id, elementary_id;
    
    int dimensions[] = {-1 , 1 , 2 , 2 , 3 , 3 , 3 , 3 , 1 , 2 , 2 , 3 , 3 , 3 , 3 , 0};
    
    char buf[1024];
    buf[0] = 0;
    line.getline(buf, 1024);

    line >> element_id;
    line >> type_id;
    line >> div_id;
    line >> physical_id;
    line >> elementary_id;
    
    if (div_id != 2 || type_id > 15) {
        std::cout << "div_id " << div_id << " type_id " << type_id << std::endl;
        DebugStop();
    }
    int dimension = dimensions[type_id];
    if (m_dim_physical_tag_and_physical_tag[dimension].find(physical_id) == m_dim_physical_tag_and_physical_tag[dimension].end())
    {
        std::cout << __PRETTY_FUNCTION__ << "physical_id " << physical_id << " not found bailing out\n";
        DebugStop();
    }
    int matid = m_dim_physical_tag_and_physical_tag[dimension][physical_id];
    
    m_entity_index.Resize(element_id);
    m_entity_index[element_id-1] = elementary_id;
    switch (type_id) {
        case 1:
        {
            // Line
            line >> TopolLine[0]; //node 1
            line >> TopolLine[1]; //node 2
            element_id--;
            TopolLine[0]--;
            TopolLine[1]--;
            if(dimension < 1)
            {
                gmesh->SetDimension(1);
                dimension = 1;
            }
            new TPZGeoElRefPattern< pzgeom::TPZGeoLinear> (element_id, TopolLine, matid, *gmesh);
        }
            break;
        case 2:
        {
            // Triangle
            line >> TopolTriangle[0]; //node 1
            line >> TopolTriangle[1]; //node 2
            line >> TopolTriangle[2]; //node 3
            element_id--;
            TopolTriangle[0]--;
            TopolTriangle[1]--;
            TopolTriangle[2]--;
            if (dimension < 2) {
                gmesh->SetDimension(2);
                dimension = 2;
            }
            new TPZGeoElRefPattern< pzgeom::TPZGeoTriangle> (element_id, TopolTriangle, matid, *gmesh);
            
        }
            break;
        case 3:
        {
            // Quadrilateral
            line >> TopolQuad[0]; //node 1
            line >> TopolQuad[1]; //node 2
            line >> TopolQuad[2]; //node 3
            line >> TopolQuad[3]; //node 4
            element_id--;
            TopolQuad[0]--;
            TopolQuad[1]--;
            TopolQuad[2]--;
            TopolQuad[3]--;
            if (dimension < 2) {
                gmesh->SetDimension(2);
                dimension = 2;
            }
            new TPZGeoElRefPattern< pzgeom::TPZGeoQuad> (element_id, TopolQuad, matid, *gmesh);
            
        }
            break;
        case 4:
        {
            // Tetrahedron
            line >> TopolTet[0]; //node 1
            line >> TopolTet[1]; //node 2
            line >> TopolTet[2]; //node 3
            line >> TopolTet[3]; //node 4
            element_id--;
            TopolTet[0]--;
            TopolTet[1]--;
            TopolTet[2]--;
            TopolTet[3]--;
            if (dimension < 3) {
                dimension = 3;
                gmesh->SetDimension(dimension);
            }
            new TPZGeoElRefPattern< pzgeom::TPZGeoTetrahedra> (element_id, TopolTet, matid, *gmesh);
            
        }
            break;
        case 5:
        {
            // Hexahedra
            line >> TopolHex[0]; //node 1
            line >> TopolHex[1]; //node 2
            line >> TopolHex[2]; //node 3
            line >> TopolHex[3]; //node 4
            line >> TopolHex[4]; //node 5
            line >> TopolHex[5]; //node 6
            line >> TopolHex[6]; //node 7
            line >> TopolHex[7]; //node 8
            element_id--;
            TopolHex[0]--;
            TopolHex[1]--;
            TopolHex[2]--;
            TopolHex[3]--;
            TopolHex[4]--;
            TopolHex[5]--;
            TopolHex[6]--;
            TopolHex[7]--;
            if (dimension < 3) {
                dimension = 3;
                gmesh->SetDimension(dimension);
            }
            new TPZGeoElRefPattern< pzgeom::TPZGeoCube> (element_id, TopolHex, matid, *gmesh);
        }
            break;
        case 6:
        {
            // Prism
            line >> TopolPrism[0]; //node 1
            line >> TopolPrism[1]; //node 2
            line >> TopolPrism[2]; //node 3
            line >> TopolPrism[3]; //node 4
            line >> TopolPrism[4]; //node 5
            line >> TopolPrism[5]; //node 6
            element_id--;
            TopolPrism[0]--;
            TopolPrism[1]--;
            TopolPrism[2]--;
            TopolPrism[3]--;
            TopolPrism[4]--;
            TopolPrism[5]--;
            if (dimension < 3) {
                dimension = 3;
                gmesh->SetDimension(dimension);
            }

            new TPZGeoElRefPattern< pzgeom::TPZGeoPrism> (element_id, TopolPrism, matid, *gmesh);
        }
            break;
        case 7:
        {
            // Pyramid
            line >> TopolPyr[0]; //node 1
            line >> TopolPyr[1]; //node 2
            line >> TopolPyr[2]; //node 3
            line >> TopolPyr[3]; //node 4
            line >> TopolPyr[4]; //node 5
            element_id--;
            TopolPyr[0]--;
            TopolPyr[1]--;
            TopolPyr[2]--;
            TopolPyr[3]--;
            TopolPyr[4]--;
            if (dimension < 3) {
                dimension = 3;
                gmesh->SetDimension(dimension);
            }
            new TPZGeoElRefPattern< pzgeom::TPZGeoPyramid> (element_id, TopolPyr, matid, *gmesh);
        }
            break;
        case 8:
        {
            // Quadratic Line
            line >> TopolLineQ[0]; //node 1
            line >> TopolLineQ[1]; //node 2
            line >> TopolLineQ[2]; //node 2
            element_id--;
            TopolLineQ[0]--;
            TopolLineQ[1]--;
            TopolLineQ[2]--;
            if (dimension < 1) {
                dimension = 1;
                gmesh->SetDimension(dimension);
            }
            new TPZGeoElRefPattern< pzgeom::TPZQuadraticLine> (element_id, TopolLineQ, matid, *gmesh);
        }
            break;
        case 9:
        {
            // Triangle
            line >> TopolTriangleQ[0]; //node 1
            line >> TopolTriangleQ[1]; //node 2
            line >> TopolTriangleQ[2]; //node 3
            line >> TopolTriangleQ[3]; //node 4
            line >> TopolTriangleQ[4]; //node 5
            line >> TopolTriangleQ[5]; //node 6
            element_id--;
            TopolTriangleQ[0]--;
            TopolTriangleQ[1]--;
            TopolTriangleQ[2]--;
            TopolTriangleQ[3]--;
            TopolTriangleQ[4]--;
            TopolTriangleQ[5]--;
            if (dimension < 2) {
                dimension = 2;
                gmesh->SetDimension(dimension);
            }
            new TPZGeoElRefPattern< pzgeom::TPZQuadraticTrig> (element_id, TopolTriangleQ, matid, *gmesh);
        }
            break;
        case 10:
        {
            // Quadrilateral
            line >> TopolQuadQ[0]; //node 1
            line >> TopolQuadQ[1]; //node 2
            line >> TopolQuadQ[2]; //node 3
            line >> TopolQuadQ[3]; //node 4
            line >> TopolQuadQ[4]; //node 5
            line >> TopolQuadQ[5]; //node 6
            line >> TopolQuadQ[6]; //node 7
            line >> TopolQuadQ[7]; //node 8
            element_id--;
            TopolQuadQ[0]--;
            TopolQuadQ[1]--;
            TopolQuadQ[2]--;
            TopolQuadQ[3]--;
            TopolQuadQ[4]--;
            TopolQuadQ[5]--;
            TopolQuadQ[6]--;
            TopolQuadQ[7]--;
            if (dimension < 2) {
                dimension = 2;
                gmesh->SetDimension(dimension);
            }
            new TPZGeoElRefPattern< pzgeom::TPZQuadraticQuad> (element_id, TopolQuadQ, matid, *gmesh);
        }
            break;
        case 11:
        {
            // Tetrahedron
            line >> TopolTetQ[0]; //node 1
            line >> TopolTetQ[1]; //node 2
            line >> TopolTetQ[2]; //node 3
            line >> TopolTetQ[3]; //node 4
            
            line >> TopolTetQ[4]; //node 4 -> 4
            line >> TopolTetQ[5]; //node 5 -> 5
            line >> TopolTetQ[6]; //node 6 -> 6
            line >> TopolTetQ[7]; //node 7 -> 7
            line >> TopolTetQ[9]; //node 9 -> 8
            line >> TopolTetQ[8]; //node 8 -> 9
            
            element_id--;
            TopolTetQ[0]--;
            TopolTetQ[1]--;
            TopolTetQ[2]--;
            TopolTetQ[3]--;
            
            TopolTetQ[4]--;
            TopolTetQ[5]--;
            TopolTetQ[6]--;
            TopolTetQ[7]--;
            TopolTetQ[8]--;
            TopolTetQ[9]--;
            if (dimension < 3) {
                dimension = 3;
                gmesh->SetDimension(dimension);
            }
            new TPZGeoElRefPattern< pzgeom::TPZQuadraticTetra> (element_id, TopolTetQ, matid, *gmesh);
            
        }
            break;
        case 12:
        {
            // Hexahedra
            line >> TopolHexQ[0]; //node 1
            line >> TopolHexQ[1]; //node 2
            line >> TopolHexQ[2]; //node 3
            line >> TopolHexQ[3]; //node 4
            line >> TopolHexQ[4]; //node 5
            line >> TopolHexQ[5]; //node 6
            line >> TopolHexQ[6]; //node 7
            line >> TopolHexQ[7]; //node 8
            
            line >> TopolHexQ[8];  //node 8  -> 8
            line >> TopolHexQ[11]; //node 11 -> 9
            line >> TopolHexQ[12]; //node 12 -> 10
            line >> TopolHexQ[9];  //node 9  -> 11
            line >> TopolHexQ[13]; //node 13 -> 12
            line >> TopolHexQ[10]; //node 10 -> 13
            line >> TopolHexQ[14]; //node 14 -> 14
            line >> TopolHexQ[15]; //node 15 -> 15
            line >> TopolHexQ[16]; //node 16 -> 16
            line >> TopolHexQ[19]; //node 19 -> 17
            line >> TopolHexQ[17]; //node 17 -> 18
            line >> TopolHexQ[18]; //node 18 -> 19
            
            element_id--;
            TopolHexQ[0]--;
            TopolHexQ[1]--;
            TopolHexQ[2]--;
            TopolHexQ[3]--;
            TopolHexQ[4]--;
            TopolHexQ[5]--;
            TopolHexQ[6]--;
            TopolHexQ[7]--;
            
            TopolHexQ[8]--;
            TopolHexQ[9]--;
            TopolHexQ[10]--;
            TopolHexQ[11]--;
            TopolHexQ[12]--;
            TopolHexQ[13]--;
            TopolHexQ[14]--;
            TopolHexQ[15]--;
            TopolHexQ[16]--;
            TopolHexQ[17]--;
            TopolHexQ[18]--;
            TopolHexQ[19]--;
            if (dimension < 3) {
                dimension = 3;
                gmesh->SetDimension(dimension);
            }
            new TPZGeoElRefPattern< pzgeom::TPZQuadraticCube> (element_id, TopolHexQ, matid, *gmesh);
        }
            break;
        case 13:
        {
            // Prism
            line >> TopolPrismQ[0]; //node 1
            line >> TopolPrismQ[1]; //node 2
            line >> TopolPrismQ[2]; //node 3
            line >> TopolPrismQ[3]; //node 4
            line >> TopolPrismQ[4]; //node 5
            line >> TopolPrismQ[5]; //node 6
            
            line >> TopolPrismQ[6];  //node 6 -> 6
            line >> TopolPrismQ[8];  //node 8 -> 7
            line >> TopolPrismQ[9];  //node 9 -> 8
            line >> TopolPrismQ[7];  //node 7 -> 9
            line >> TopolPrismQ[10]; //node 11 -> 10
            line >> TopolPrismQ[11]; //node 12 -> 11
            line >> TopolPrismQ[12]; //node 13 -> 12
            line >> TopolPrismQ[14]; //node 14 -> 13
            line >> TopolPrismQ[13]; //node 15 -> 14
            
            element_id--;
            TopolPrismQ[0]--;
            TopolPrismQ[1]--;
            TopolPrismQ[2]--;
            TopolPrismQ[3]--;
            TopolPrismQ[4]--;
            TopolPrismQ[5]--;
            
            TopolPrismQ[6]--;
            TopolPrismQ[7]--;
            TopolPrismQ[8]--;
            TopolPrismQ[9]--;
            TopolPrismQ[10]--;
            TopolPrismQ[11]--;
            TopolPrismQ[12]--;
            TopolPrismQ[13]--;
            TopolPrismQ[14]--;
            if (dimension < 3) {
                dimension = 3;
                gmesh->SetDimension(dimension);
            }

            new TPZGeoElRefPattern< pzgeom::TPZQuadraticPrism> (element_id, TopolPrismQ, matid, *gmesh);
        }
            break;
        case 15:{
            // Point
            line >> TopolPoint[0];
            TopolPoint[0]--;
            element_id--;
            new TPZGeoElement< pzgeom::TPZGeoPoint, pzrefine::TPZRefPoint> (element_id, TopolPoint, matid, *gmesh);
        }
            break;
        default:
        {
            std::cout << "Element not impelemented." << std::endl;
            DebugStop();
        }
            break;
    }
    
    return true;
}
