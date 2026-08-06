/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "VcMeshUtils.h"
#include "AppToolkitIJson.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <random>
#include <cmath>
#include <cstring>
#include <optional>
#include <sstream>

namespace SwApi
{
namespace VcMeshUtils
{

using namespace std;

const int STEP_MESH_H_START_OFF = 0x1;
const int STEP_MESH_H_BIT_SIZE = 0x2;
#define SET_STEP_MESH_H_STR(pip) (((0x1) << (STEP_MESH_H_BIT_SIZE * pip)) << STEP_MESH_H_START_OFF)
#define SET_STEP_MESH_H_COMP(pip) (((0x2) << (STEP_MESH_H_BIT_SIZE * pip)) << STEP_MESH_H_START_OFF)
#define SET_STEP_MESH_V_STR (0x1) // This is a pointless wrapper at the moment. Useful if we ever change the V stretch bits though.

optional<VcCpMesh> ReadCpMeshFromBin(istream& f)
{
    VcCpMesh result;

    // Go through line-by-line.
    string line;
    while (getline(f, line)) {
        // Read each line into an encoded format.
        uint32_t i = 0;
        try {
            i = stoul(line, nullptr, 2);
        } catch (exception const& _) {
            // Failed to parse. Report the error.
            return {};
        }
        result.push_back(i);
    }

    return result;
}


optional<VcCpFloatMesh> ReadSingleCpFloatMeshFromJson(AtUtils::IJsonValuePtr json)
{
    VcCpFloatMesh result;

    auto json_array = json->GetArray();

    if(!json_array)
        return {};

    for(std::size_t line = 0; line < json_array->Size(); ++line)
    {
        const auto j = json_array->At(line)->GetArray();

        if(!j) return {};

        for(std::size_t entry = 0; entry < j->Size(); ++entry)
        {
            const auto jv = (*j)[entry]->GetValue();
            float v = 0.0f;

            // JSON parser will read 1 as uint32_t
            // -1 as int32_t
            // 1.0 as double

            if(std::holds_alternative<int32_t>(jv))
                v = static_cast<float>(std::get<int32_t>(jv));
            else if(std::holds_alternative<uint32_t>(jv))
                v = static_cast<float>(std::get<uint32_t>(jv));
            else if(std::holds_alternative<double>(jv))
                v = static_cast<float>(std::get<double>(jv));
            else
                return {};

            result.push_back(v);
        }
    }

    return result;
}


std::optional<AtUtils::IJsonValuePtr> findObjectEntryByName(AtUtils::IJsonObjectPtr json, const char* name)
{
    for(std::size_t i = 0; i < json->GetNumMembers(); ++i)
    {
        const auto j = json->GetMember(i);

        if(j->GetName() == name)
            return j->GetValue();
    }

    return {};
}

std::optional<AtUtils::IJsonValuePtr> findObjectEntryByNames(AtUtils::IJsonObjectPtr json, const vector<const char*>& names)
{
    for (auto name : names){
        auto foundObject = findObjectEntryByName(json, name);
        if (foundObject) {
            return foundObject;
        }
    }
    return {};
}


optional<vector<VcCpFloatMesh>> ReadCpFloatMeshesFromJson(AtUtils::IJsonObjectPtr json)
{
    if(!json)
        return {};

    int numColorPlanes = json->GetNumMembers();

    if((numColorPlanes == 0) or (numColorPlanes > 4))
        return {};

    vector<VcCpFloatMesh> result;

    using colourplane_name_map_t = std::map<int, std::vector<const char*>>;

    static const std::array<colourplane_name_map_t, 4> colorPlaneNames{{
        {{ 0, {"Grey", "Gray"} }},
        {{ 0, {"R"} }, { 1, {"G"} }},
        {{ 0, {"R"} }, { 1, {"G"} }, { 2, {"B"} }},
        {{ 0, {"R"} }, { 1, {"Gr"} }, { 2, {"Gb"} },{ 3, {"B"} }}
    }};    

    const auto& colourPlaneNameMap = colorPlaneNames[numColorPlanes - 1];

    for(int colorPlane = 0; colorPlane < numColorPlanes; colorPlane++)
    {
        const auto& colourPlaneName = colourPlaneNameMap.at(colorPlane);

        auto entryFound = findObjectEntryByNames(json, colourPlaneName);

        if (not entryFound) {
            return {};
        }

        optional<VcCpFloatMesh> mesh = ReadSingleCpFloatMeshFromJson(*entryFound);
        if (not mesh) {
            return {};
        }

        result.push_back(*mesh);
    }

    return result;
}


optional<vector<VcCpMesh>> ReadCpMeshesFromJson(istream& f)
{
    const auto json = AtUtils::IJson::Create(f);

    if(!json)
        return {};
    
    const auto json_root = json->Parse();

    if(!json_root)
        return {};

     optional<vector<VcCpFloatMesh>> floatMeshes = ReadCpFloatMeshesFromJson(std::move(json_root));

     if (floatMeshes) {
         vector<VcCpMesh> meshes;
         for (const auto& floatMesh : *floatMeshes) {
             meshes.push_back(quantizeMeshToFixedPoint8i11f(floatMesh));
         }

         return {meshes};
     } else {
         return {};
     }
}

/// Reads meshes from the input `f`, first viewed as a bitstring, then attempting JSON.
optional<vector<VcCpMesh>> ReadCpMeshesFrom(istream& f)
{
    optional<VcCpMesh> meshesFromBin = ReadCpMeshFromBin(f);

    if (meshesFromBin.has_value()) {
        return {vector<VcCpMesh>{*meshesFromBin}};
    }

    optional<vector<VcCpMesh>> meshesFromJson = ReadCpMeshesFromJson(f);

    if (meshesFromJson.has_value()) {
        return meshesFromJson;
    }

    return {};
}

optional<VcStepMesh> ReadStepMeshFromBin(istream& f)
{
    VcStepMesh result;

    // Go through line-by-line.
    std::string line;
    while (std::getline(f, line)) {
        // Read line in, viewed as a 16 bit bitstring.
        uint32_t i = 0;
        try {
            i = std::stoul(line, nullptr, 2);
        } catch (exception const& _) {
            return {};
        }
        result.push_back(i);
    }

    return result;
}

std::pair<VcStepMesh, VcStepSampleCoordMesh> GenerateStepMesh(uint16_t resX, uint16_t resY, uint8_t pip, uint16_t meshX, uint16_t meshY)
{
    uint16_t h_num_blocks = meshX - 1;
    uint16_t v_num_blocks = meshY - 1;

    uint16_t pix_block_count = ((resX / h_num_blocks) / pip) * pip;
    uint16_t block_line_count = resY / v_num_blocks;

    uint16_t horiz_pix_diff = resX - (pix_block_count * h_num_blocks);
    uint16_t vert_line_diff = resY - (block_line_count * v_num_blocks);

    uint32_t stepMesh[meshY][meshX];
    uint32_t gridSizeX[meshX];
    uint32_t gridSizeY[meshY];

    // The algorithms tend to start from 1, this is a precaution
    stepMesh[0][0] = 0;
    gridSizeX[0] = 0;
    gridSizeY[0] = 0;

    for (uint16_t i = 1; i < meshX; i++)
    {
        stepMesh[0][i] = 0;
        gridSizeX[i] = pix_block_count;
    }
    for (uint16_t i = 1; i < meshY; i++)
    {
        gridSizeY[i] = block_line_count;
    }

    /*
    std::cout << "h_num_blocks: " << h_num_blocks << "\n";
    std::cout << "v_num_blocks: " << v_num_blocks << "\n";
    std::cout << "pix_block_count: " << pix_block_count << "\n";
    std::cout << "horiz_pix_diff: " << horiz_pix_diff << "\n";
    std::cout << "vert_line_diff: " << vert_line_diff << "\n";
    */

    // For generating the horizontal lines
    // First, stretch the last zone to account for the h_comp bits and the non-even
    // amount of pip stretching. Only do this check if horiz_pix_diff is not a multiple of pip
    // Then do the rest of the stretches will all the pip instances
    // When done, memcpy onto the other lines
    if (horiz_pix_diff)
    {
        uint16_t remaining_pix = horiz_pix_diff;
        uint16_t num_horiz_stretches = 1 + ((horiz_pix_diff - 1) / pip);
        uint16_t max_stretch_index = h_num_blocks - 1;

        uint16_t non_even_last_col_stretch = remaining_pix % pip;
        //std::cout << "non_even_last_col_stretch: " << non_even_last_col_stretch << "\n";
        if (non_even_last_col_stretch)
        {
            //std::cout << "Compensating for non-even last column stretch\n";
            for (int i = 0; i < non_even_last_col_stretch; i++)
            {
                stepMesh[0][max_stretch_index] |= SET_STEP_MESH_H_STR(i);
                //std::cout << "stepMesh[0][" << max_stretch_index << "]: " << stepMesh[0][max_stretch_index] << "\n";
            }
            for (int i = 0; i < pip; i++)
            {
                stepMesh[0][max_stretch_index] |= SET_STEP_MESH_H_COMP(i);
            }
            gridSizeX[max_stretch_index] = pix_block_count + non_even_last_col_stretch;

            max_stretch_index -= 1;
            remaining_pix -= non_even_last_col_stretch;
            num_horiz_stretches -= 1;
        }

        // No pesky divide by zero errors here, thank you.
        if (num_horiz_stretches)
        {
            //std::cout << "num_horiz_stretches remaining: " << num_horiz_stretches << "\n";
            float step_itr = (float)num_horiz_stretches / (max_stretch_index - 1); // -1 because we don't stretch the first column 
            float step_float_counter = 0;
            uint8_t step_int_counter = 0;
            // Note, we start from column ONE in this algorithm, not ZERO.
            for (int i = max_stretch_index; i >= 1; i--)
            {
                step_float_counter += step_itr; // TODO - check if this needs to be above the if statement
                //std::cout << "step_float_counter: " << step_float_counter << "\n";
                //std::cout << "step_int_counter: " << step_int_counter << "\n";
                if ((int)step_float_counter > step_int_counter)
                {
                    //std::cout << "Threshold met, applying stretch to column " << i << "\n";
                    step_int_counter++;
                    for (int j = 0; j < pip; j++)
                    {
                        stepMesh[0][i] |= SET_STEP_MESH_H_STR(j);
                        if (i == (h_num_blocks - 1))
                        {
                            stepMesh[0][i] |= SET_STEP_MESH_H_COMP(j);
                        }
                    }
                    gridSizeX[i] += pip;
                    remaining_pix -= pip; // This is more for error tracking at this point
                    num_horiz_stretches -= 1;
                }

                if (num_horiz_stretches == 0)
                {
                    // If the formula breaks, then at least this'll stop us overstretching
                    break;
                }
            }
        }

        //std::cout << "Remaining pix unaccounted for: " << remaining_pix << "\n";
        //std::cout << "Remaining stretches: " << num_horiz_stretches << "\n";
    }

    for (uint16_t i = 1; i < meshY; i++)
    {
        if (i < meshY - 1)
        {
            memcpy(stepMesh[i], stepMesh[0], meshX * sizeof(uint32_t));
        }
        else
        {
            memset(stepMesh[i], 0, meshX * sizeof(uint32_t));
        }
    }

    /*
    std::cout << "gridSizeX\n";
    for (int i = 0; i < meshX; i++)
    {
        std::cout << gridSizeX[i] << ", ";
    }
    std::cout << "\n";
    */

    if (vert_line_diff)
    {
        uint16_t max_stretch_index = v_num_blocks - 1;
        uint16_t remaining_lines = vert_line_diff;

        float step_itr = (float)vert_line_diff / (max_stretch_index - 1); // -1 because we don't stretch the first line. TODO - is this valid for all cases?
        float step_float_counter = 0;
        uint8_t step_int_counter = 0;

        for (int i = 1; i <= max_stretch_index; i++)
        {
            step_float_counter += step_itr;

            if ((int)step_float_counter > step_int_counter)
            {
                step_int_counter++;
                for (int j = 0; j < h_num_blocks; j++)
                {
                    stepMesh[i][j] |= SET_STEP_MESH_V_STR;
                }
                gridSizeY[i] += 1;
                remaining_lines -= 1; // This is more for error tracking at this point
            }

            if (remaining_lines == 0)
            {
                // TODO - the fact that this is necessary implies there's an issue with the formula
                // and it should need re-looking at again. For now, it works and generates an appropriate UV mesh for sampling
                break;
            }
        }

        //std::cout << "Remaining lines: " << remaining_lines << "\n";
    }

    /*
    std::cout << "gridSizeY\n";
    for (int i = 0; i < meshY; i++)
    {
        std::cout << gridSizeY[i] << ", ";
    }
    std::cout << "\n";
    */

    VcStepMesh result;

    for (uint16_t y = 0; y < meshY; y++)
    {
        for (uint16_t x = 0; x < meshX; x++)
        {
            result.push_back(stepMesh[y][x]);
        }
    }

    VcStepSampleCoordMesh coords;
    float x_accumulator = 0;
    float y_accumulator = 0;
    for (uint16_t y = 0; y < meshY; y++)
    {
        y_accumulator += gridSizeY[y];
        coords.push_back(y_accumulator / resY);
    }
    for (uint16_t x = 0; x < meshX; x++)
    {
        x_accumulator += gridSizeX[x];
        coords.push_back(x_accumulator / resX);
    }

    return std::make_pair(result, coords);
}


template<typename T>
static inline T max(T a, T b)
{
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

/// Returns a random number in the range [a, b]
unsigned int randRange(unsigned int a, unsigned int b)
{
    std::random_device dev;
    std::mt19937 mt(dev());
    std::uniform_int_distribution<unsigned int> dist(a,b);
    return dist(mt);
}

void addVignetteEffectTo(VcCpFloatMesh& to,
                         unsigned int hMeshPoints,
                         unsigned int vMeshPoints,
                         MeshGenParameters params)
{
    auto index = [hMeshPoints](unsigned int x, unsigned int y) { return y * hMeshPoints + x; };

    unsigned int maxVignetteWidth = max(1u, hMeshPoints / 3);
    unsigned int maxVignetteHeight = max(1u, vMeshPoints / 3);

    // Left vignette
    unsigned int leftVignette_width = max(1u, randRange(0, maxVignetteWidth));
    float leftVignette_initialValue = (float)(randRange(1, 256 - leftVignette_width)) / 256.0;
    float leftVignette_inc = (1.0 - leftVignette_initialValue) / leftVignette_width;

    for (unsigned int y = 0; y < vMeshPoints; y++) {
        for (unsigned int x = 0; x < leftVignette_width; x++) {
            to[index(x, y)] = leftVignette_initialValue + (x * leftVignette_inc);
        }
    }

    // Right vignette
    unsigned int rightVignette_width = max(1u, randRange(0, maxVignetteWidth));
    float rightVignette_initialValue = (float)(randRange(1, 256 - rightVignette_width)) / 256.0;
    float rightVignette_inc = (1.0 - rightVignette_initialValue) / rightVignette_width;

    for (unsigned int y = 0; y < vMeshPoints; y++) {
        for (unsigned int x = hMeshPoints - rightVignette_width - 1; x < hMeshPoints; x++) {
            to[index(x, y)]
                = rightVignette_initialValue + ((hMeshPoints - x - 1) * rightVignette_inc);
        }
    }

    // Top vignette
    unsigned int topVignette_height = max(1u, randRange(0, maxVignetteHeight));
    float topVignette_initialValue = (float)(randRange(1, 256 - topVignette_height)) / 256.0;
    float topVignette_inc = (1.0 - topVignette_initialValue) / topVignette_height;

    for (unsigned int y = 0; y < topVignette_height; y++) {
        for (unsigned int x = 0; x < hMeshPoints; x++) {
            to[index(x, y)] *= topVignette_initialValue + (y * topVignette_inc);
        }
    }

    // Bottom vignette
    unsigned int bottomVignette_height = max(1u, randRange(0, maxVignetteHeight));
    float bottomVignette_initialValue = (float)(randRange(1, 256 - bottomVignette_height)) / 256.0;
    float bottomVignette_inc = (1.0 - bottomVignette_initialValue) / bottomVignette_height;

    for (unsigned int y = vMeshPoints - bottomVignette_height - 1; y < vMeshPoints; y++) {
        for (unsigned int x = 0; x < hMeshPoints; x++) {
            to[index(x, y)]
                *= bottomVignette_initialValue + ((vMeshPoints - y - 1) * bottomVignette_inc);
        }
    }
}

void addLensCoronaEffectTo(VcCpFloatMesh& to,
                           unsigned int hMeshPoints,
                           unsigned int vMeshPoints,
                           MeshGenParameters params)
{
    auto index = [hMeshPoints](unsigned int x, unsigned int y) { return y * hMeshPoints + x; };

    unsigned int lensCentreX = hMeshPoints / 2;
    unsigned int lensCentreY = vMeshPoints / 2;

    float lensRadius = params.lensRadius;

    float lensCentre_strength = 1.0f + ((randRange(0, 100) / 100.0) * params.maxMeshIntensity);
    float lensCentre_dec = (lensCentre_strength - 1.0) / lensRadius;

    for (unsigned int y = 0; y < vMeshPoints; y++) {
        for (unsigned int x = 0; x < hMeshPoints; x++) {
            auto dist = sqrt((lensCentreX - x) * (lensCentreX - x)
                             + (lensCentreY - y) * (lensCentreY - y));
            if (dist < lensRadius) {
                to[index(x, y)] *= lensCentre_strength - (dist * lensCentre_dec);
            }
        }
    }
}

VcCpMesh quantizeMeshToFixedPoint8i11f(const VcCpFloatMesh& from)
{
    VcCpMesh result;
    result.reserve(from.size());

    std::ranges::transform(from, std::back_inserter(result), ToFixedPoint<8,11>);

    return result;
}

array<VcCpFloatMesh, 4> SplitIntensityMeshIntoBayerChannels(const VcCpFloatMesh& intensities)
{
    array<VcCpFloatMesh, 4> result;

    // Conversion coefficients based on the Photometric Digital ITU BT709 spec.
    //float rCoeff = 0.2126;
    //float grCoeffs = (0.7152/2);
    //float gbCoeffs = (0.7152/2);
    //float bCoeffs = 0.0722;
    // Conversion coefficients based on the Rec 601 spec.
    float rCoeff = 0.299;
    float grCoeffs = (0.587/2);
    float gbCoeffs = (0.587/2);
    float bCoeffs = 0.144;

    for (auto i : intensities) {
        // result[0].push_back(rCoeff * i);
        // result[1].push_back(grCoeffs * i);
        // result[2].push_back(gbCoeffs * i);
        // result[3].push_back(bCoeffs * i);

        result[0].push_back(rCoeff * (i - 1.0) + 1.0);
        result[1].push_back(grCoeffs * (i - 1.0) + 1.0);
        result[2].push_back(gbCoeffs * (i - 1.0) + 1.0);
        result[3].push_back(bCoeffs * (i - 1.0) + 1.0);
    }

    return result;
}

VcCpFloatMesh GenerateFloatMeshFor(unsigned int hMeshPoints,
                                   unsigned int vMeshPoints,
                                   MeshGenParameters params)
{
    VcCpFloatMesh floatMesh;
    for (unsigned int i = 0; i < hMeshPoints * vMeshPoints; i++) {
        floatMesh.push_back(1.0f);
    }

    addVignetteEffectTo(floatMesh, hMeshPoints, vMeshPoints, params);
    addLensCoronaEffectTo(floatMesh, hMeshPoints, vMeshPoints, params);

    VcCpFloatMesh floatMeshRescaled;
    for (unsigned int i = 0; i < hMeshPoints * vMeshPoints; i++) {
        float val = floatMesh[i];
        float scaledVal = clamp((min(val, 1.0f) - 1.0) * 0.01 * params.strength, -0.5, 3.0);
        floatMeshRescaled.push_back(1.0+scaledVal);
    }

    return floatMesh;
}

VcCpFloatMesh GenerateUnityFMesh(unsigned int hMeshPoints,
                           unsigned int vMeshPoints)
{
    VcCpFloatMesh floatMesh;

    for (unsigned int i = 0; i < hMeshPoints * vMeshPoints; i++) {
        floatMesh.push_back(1.0f);
    }

    return floatMesh;
}

VcCpMesh GenerateUnityMesh(unsigned int hMeshPoints,
                           unsigned int vMeshPoints,
                           int bitcnt)
{
    VcCpMesh unityMesh;

    for (unsigned int i = 0; i < hMeshPoints * vMeshPoints; i++) {
        unityMesh.push_back(1u << bitcnt);
    }

    return unityMesh;
}

VcCpMesh GenerateMeshFor(unsigned int hMeshPoints,
                         unsigned int vMeshPoints,
                         MeshGenParameters params)
{
    VcCpFloatMesh floatMesh = GenerateFloatMeshFor(hMeshPoints, vMeshPoints, params);

    VcCpMesh result = quantizeMeshToFixedPoint8i11f(floatMesh);

    return result;
}

VcCpFloatMesh GenerateMirroredFloatMeshFor(unsigned int hMeshPoints,
                                           unsigned int vMeshPoints,
                                           MeshGenParameters params)
{
    VcCpFloatMesh floatMesh = GenerateFloatMeshFor(hMeshPoints, vMeshPoints, params);

    VcCpFloatMesh mirroredMesh = MirrorMesh(floatMesh);

    return mirroredMesh;
}

VcCpMesh GenerateMirroredMeshFor(unsigned int hMeshPoints,
                                 unsigned int vMeshPoints,
                                 MeshGenParameters params)
{
    VcCpFloatMesh floatMesh = GenerateMirroredFloatMeshFor(hMeshPoints, vMeshPoints, params);

    VcCpMesh result = quantizeMeshToFixedPoint8i11f(floatMesh);

    return result;
}

float maxEntryIn(const VcCpFloatMesh& mesh)
{
    float maxThusFar = 0.0f;
    for (float f : mesh) {
        if (f > maxThusFar) {
            maxThusFar = f;
        }
    }
    return maxThusFar;
}

VcCpFloatMesh MirrorMesh(const VcCpFloatMesh& mesh, float max)
{
    // Numbers in the range [1.0f, max] go to (0.0f, 1.0f)
    // Numbers in the range (0.0f, 1.0f) go to [1.0f, max]

    VcCpFloatMesh result;
    if (max < 0.0f) {
        max = maxEntryIn(mesh);
    }
    for (float f : mesh) {
        double mirrored;
        if (f < 1.0f) {
            double ratio = 1.0 - f;
            mirrored = 1.0 + ratio * (max - 1.0);
        } else {
            double ratio = (f - 1.0) / (max - 1.0);
            mirrored = 1.0 - ratio;
        }
        result.push_back(mirrored);
    }

    return result;
}

std::string formatAsBinary(uint32_t x, unsigned int bitcnt)
{
    std::stringstream ss;
    for (int bitidx = bitcnt - 1; bitidx >= 0; bitidx--) {
        bool bit = x & (1 << bitidx);
        if (bit) {
            ss << '1';
        } else {
            ss << '0';
        }
    }
    return ss.str();
}

std::ostream& PrintInBinary(std::ostream& os, const VcCpMesh& mesh, const char* sep)
{
    for (auto i : mesh) {
        os << formatAsBinary(i, 19) << sep;
    }
    return os;
}

std::ostream& PrintAsTable(std::ostream& os, const VcCpMesh& mesh, unsigned int hMeshPoints)
{
    os << "VcCpMesh(";
    bool first = true;
    unsigned int idx = 0;
    for (auto i : mesh) {
        if (not first) {
            os << ", ";
        }
        first = false;

        if (idx == hMeshPoints) {
            os << '\n';
            idx = 0;
        }

        idx++;

        os << i;
    }
    os << ")";
    return os;
}

std::ostream& operator<<(std::ostream& os, const MeshGenParameters& params)
{
    // clang-format off
    return os
        << "MeshGenParameters("
        << "maxMeshIntensity: " << params.maxMeshIntensity
        << ", "
        << "lensRadius: " << params.lensRadius
        << ")";
    // clang-format on
}

std::string ToString(const MeshGenParameters& params)
{
    std::stringstream ss;
    ss << params;
    return ss.str();
}

std::ostream& operator<<(std::ostream& os, const VcCpMesh& mesh)
{
    os << "VcCpMesh(";
    bool first = true;
    for (auto i : mesh) {
        if (not first) {
            os << ", ";
        }
        os << i;
        first = false;
    }
    os << ")";
    return os;
}

std::string ToString(const VcCpMesh& mesh)
{
    std::stringstream ss;
    ss << mesh;
    return ss.str();
}

} // namespace VcMeshUtils
} // namespace SwApi
