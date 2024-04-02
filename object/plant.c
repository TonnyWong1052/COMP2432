#include <string.h>

struct Plant {
    char name[50];
    int productiveForces;
};

void setPlantAttributes(struct Plant* plant, const char* name, int productiveForces) {
    strcpy(plant->name, name);
    plant->productiveForces = productiveForces;
}