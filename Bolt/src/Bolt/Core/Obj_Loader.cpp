#include "Obj_Loader.h"
#include "Assert.h"
#include <glm/glm.hpp>
#include <iostream>


std::vector<std::pair<std::string, std::string>> Bolt::Obj_Loader::Load(const std::string& sub_folder, const std::string& file_name, Assets_Resource_Injector* injector)
{
    
    std::vector<std::pair<std::string, std::string>> shape_names;

    std::unordered_map<std::string, Model_Loader::Mesh_Data> sub_meshes;

    Model_Loader::Mesh_Data* active_sub_mesh = nullptr;
    std::string active_object_name;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texture_coord;

   
    std::ifstream file(Full_Path(sub_folder.c_str(), file_name.c_str(), File_Suffix()));
    if (!file.is_open())
        ERROR("failed to open file!");
    
    //https://en.wikipedia.org/wiki/Wavefront_.obj_file

    while (!file.eof())
    {
        std::string word;
        std::getline(file, word, ' ');
        WORD_CHECKS_START:

        //Comment, ignore the line. ...or unhandled entry type.
        if (word == "#" || word == "" || word == "g" || word == "s" || word == "vp" || word == "u" || word == "w" || word == "l")
        {
            file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        //Push recorded mesh and start recording a new mesh!
        if (word == "o")
        {
            Push_Sub_Meshes(sub_meshes, shape_names, active_object_name, injector);

            std::getline(file, word, '\n');
            active_object_name = file_name + File_Suffix() + "/" + word;
            continue;
        }

        if (word == "mtllib")
        {
            std::getline(file, word, '\n');

            if (m_parsed_mtllib_files.insert(word).second)
                Load_mtl(file_name, sub_folder + word, injector);

            continue;
        }

        if (word == "usemtl")
        {
            std::getline(file, word, '\n');
            active_sub_mesh = &sub_meshes[word];
            continue;
        }
            
        if (word == "v") // position (x y z)
        {
            word = Read_Position_Or_Normal_In_Tight_Loop(file, "v", positions);
            goto WORD_CHECKS_START;
        }

        if (word == "vn") // normal (x y z)
        {
            word = Read_Position_Or_Normal_In_Tight_Loop(file, "vn", normals);
            goto WORD_CHECKS_START;
        }

        if (word == "vt") // texture coordinates (x y)
        {
            word = Read_Texture_Coordinate_In_Tight_Loop(file, texture_coord);
            goto WORD_CHECKS_START;
        }

        if (word == "f") // face
        {
            ASSERT(active_sub_mesh, "Active sub mesh is a nullptr!");
            word = Read_Faces_In_Tight_Loop(file, positions, texture_coord, normals, *active_sub_mesh);
            goto WORD_CHECKS_START;
        }
        

        ERROR("Failed reading the file!");
    }
    
    file.close();

    Push_Sub_Meshes(sub_meshes, shape_names, active_object_name, injector);
    
    return shape_names;
}

void Bolt::Obj_Loader::Push_Sub_Meshes(std::unordered_map<std::string, Model_Loader::Mesh_Data>& sub_meshes, std::vector<std::pair<std::string, std::string>>& shape_names, const std::string& active_object_name, Assets_Resource_Injector* injector)
{
    uint32_t index = 0;
    for (auto& [material_name, sub_mesh] : sub_meshes)
    {
        std::pair<std::string, std::string>& mesh_and_material_name = shape_names.emplace_back();
        mesh_and_material_name.first = active_object_name + "_" + std::to_string(index++);
        mesh_and_material_name.second = material_name;

        injector->Push_Mesh(mesh_and_material_name.first, sub_mesh.vertices, sub_mesh.indices);
    }
    sub_meshes.clear();
}

std::string Bolt::Obj_Loader::Read_Faces_In_Tight_Loop(std::ifstream& file, std::vector<glm::vec3>& positions, std::vector<glm::vec2>& texture_coord, std::vector<glm::vec3>& normals, Model_Loader::Mesh_Data& output_mesh)
{
    std::string word;

    do
    {
        constexpr uint32_t VERTICES_PER_FACE = 3;
        constexpr uint32_t VERTEX_DATA_MEMBER_COUNT = 3;

        //A (supported) face consists of 3 vertices.
        //Each vertex has a postion, texture coordinate and a normal.
        for (uint32_t vert_index = 0; vert_index < VERTICES_PER_FACE; vert_index++)
        {
            std::array<uint32_t, VERTEX_DATA_MEMBER_COUNT> face_indecies = { 0, 0, 0 };

            for (uint32_t i = 0; i < face_indecies.size(); i++)
                Read_Face(file, word, face_indecies, i, vert_index);

            Vertex vertex;

            //format vertex_index/texture_index/normal_index for which each index starts at 1. 
            vertex.position = positions[face_indecies[0]];
            vertex.uv = texture_coord[face_indecies[1]];
            vertex.normal = normals[face_indecies[2]];

            auto find = output_mesh.unique_vertices.find(vertex);

            if (find != output_mesh.unique_vertices.end())
                output_mesh.indices.push_back(find->second);
            else
            {
                output_mesh.indices.push_back((uint32_t)output_mesh.vertices.size());
                output_mesh.vertices.push_back(vertex);

                output_mesh.unique_vertices[vertex] = output_mesh.indices.back();
            }
        }

        std::getline(file, word, ' ');
        if (word != "f") break;

    } while (!file.eof());

    return word;
}


const char* Bolt::Obj_Loader::File_Suffix()
{
    return ".obj";
}


std::string Bolt::Obj_Loader::Full_Path(const char* sub_folder, const char* file_name, const char* file_suffix)
{
    return std::string(sub_folder) + "/" + file_name + file_suffix;
}


void Bolt::Obj_Loader::Load_mtl(const std::string& model_name, const std::string& file_name, Assets_Resource_Injector* injector)
{
    std::ifstream file(file_name);
    if (!file.is_open())
        ERROR("failed to open file!");

    std::string word;
    std::array<char, 2> delims = { ' ', '\n' };

    std::string material_name;
    Material_Data material_data;

    while (!file.eof())
    {
        char hit = Parsing::Read_Until_Delim(file, word, delims.data(), (uint32_t)delims.size());

        //Comment, ignore the line. ...or unhandled entry type.
        if (word == "#" || word == "")
        {
            if(hit != '\n')
                file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        if (word == "newmtl")
        {
            if (!material_name.empty())
                injector->Push_Material(model_name, material_name, material_data);

            material_data = {};
            
            char new_line = '\n';
            hit = Parsing::Read_Until_Delim(file, material_name, &new_line);
            continue;
        }

        //Specular exponent
        if (word == "Ns")
        {
            char new_line = '\n';
            hit = Parsing::Read_Until_Delim(file, word, &new_line);
            material_data.specular_exponent = std::stof(word);
            continue;
        }

        if (word == "map_Kd") //diffuse texture.
        {
            char new_line = '\n';
            hit = Parsing::Read_Until_Delim(file, word, &new_line);
            auto iter = std::remove_if(word.begin(), word.end(), std::isspace);
            word.erase(iter, word.end());
            material_data.m_deffuse_texture_name = word;
            continue;
        }
    }

    if (!material_name.empty())
        injector->Push_Material(model_name, material_name, material_data);

    file.close();
}


std::string Bolt::Obj_Loader::Read_Position_Or_Normal_In_Tight_Loop(std::ifstream& file, const char* identifier, std::vector<glm::vec3>& ouput)
{
    Read_Vec3(file, ouput.emplace_back());

    while (!file.eof())
    {
        std::string word;
        std::getline(file, word, ' ');

        if (word == identifier)
            Read_Vec3(file, ouput.emplace_back());
        else
            return word;
    }

    return std::string();
}

std::string Bolt::Obj_Loader::Read_Texture_Coordinate_In_Tight_Loop(std::ifstream& file, std::vector<glm::vec2>& ouput)
{
    Read_Vec2(file, ouput.emplace_back());

    while (!file.eof())
    {
        std::string word;
        std::getline(file, word, ' ');

        if (word == "vt")
            Read_Vec2(file, ouput.emplace_back());
        else
            return word;
    }

    return std::string();
}


void Bolt::Obj_Loader::Read_Vec3(std::ifstream& file, glm::vec3& vec3)
{
    for (uint32_t i = 0; i < 3; i++)
        file >> vec3[i];
    

    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void Bolt::Obj_Loader::Read_Vec2(std::ifstream& file, glm::vec2& vec2)
{
    file >> vec2[0];
    file >> vec2[1];

    vec2[1] *= -1;

    file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void Bolt::Obj_Loader::Read_Face(std::ifstream& file, std::string& word, std::array<uint32_t, 3>& vertex_elements, uint32_t vert_member, uint32_t face_index)
{
    if (vert_member < 2)
        std::getline(file, word, '/');
    else if(face_index < 2)
        std::getline(file, word, ' ');
    else 
        std::getline(file, word, '\n');
    
    if (word.length() > 0)
        vertex_elements[vert_member] = std::stoi(word) - 1;
}

void Bolt::Parsing::Extract_File_Extension_And_Name_From_CSTR(const char* cstr, std::string& extension, std::string& path)
{
    int32_t file_name_lenght = (uint32_t)strnlen_s(cstr, std::numeric_limits<int32_t>::max());
    int32_t dot = file_name_lenght - 1;
    for (int32_t i = file_name_lenght - 1; i >= 0; i--)
        if (cstr[i] == '.')
        {
            dot = i;
            break;
        }

    if (dot == file_name_lenght - 1 || dot == 0)
        ERROR(std::string("Could not parse [ ") + cstr + " ] extension and name!");

    for (int32_t i = 0; i < file_name_lenght; i++)
        if (i < dot)
            path += cstr[i];
        else if (i > dot)
            extension += cstr[i];
}

char Bolt::Parsing::Read_Until_Delim(std::ifstream& file, std::string& output, const char* delim, uint32_t delim_count)
{
    output.clear();
    char c;
    while (!file.eof())
    {
        c = file.get();
        for(uint32_t i = 0; i < delim_count; i++)
            if (c == delim[i])
                return delim[i];
        
        output += c;
    }

    return 0;
}

