#include "obj_loader.h"

bool loadOBJFile(const char * path, std::vector<glm::vec3> & out_vertices
){
	printf("Attempting to load OBJ File from %s\n", path);

	std::vector<unsigned int> vertIndices;
	std::vector<glm::vec3> temp_vertices;

	FILE *obj_file = fopen(path, "r");
	if( obj_file == nullptr){
		printf("Error while attempting to obj file\n");
		getchar();
		return false;
	}

    int res = 0;

    while( res != EOF ){
		char lineHeader[128];
		// Read first word from line
		res = fscanf(obj_file, "%s", lineHeader);

        //Begin parsing
		if ( strcmp( lineHeader, "v" ) == 0 ){
			glm::vec3 vertex{};
			fscanf(obj_file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
			temp_vertices.push_back(vertex);
		} else if ( strcmp( lineHeader, "f " ) == 0 ){
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3];
			int matches = fscanf(obj_file, "%d %d %d \n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);

			if (matches != 3){
				printf("File unable to be parsed in current parser version\n");
				fclose(obj_file);
				return false;
			}
			vertIndices.push_back(vertexIndex[0]);
			vertIndices.push_back(vertexIndex[1]);
			vertIndices.push_back(vertexIndex[2]);
		}else{
			// Discard the rest of line if not in obj spec
			char garbage_buffer[1000];
			fgets(garbage_buffer, 1000, obj_file);
		}

	}
    // For each vertex of each triangle
    for( unsigned int i=0; i < vertIndices.size(); i++ ) {
        // Get the indices of its attributes
        unsigned int vertexIndex = vertIndices[i];
        // Get the attributes thanks to the index
        glm::vec3 vertex = temp_vertices[vertexIndex - 1];
        out_vertices.push_back(vertex);
    }
        fclose(obj_file);
	return true;
}

