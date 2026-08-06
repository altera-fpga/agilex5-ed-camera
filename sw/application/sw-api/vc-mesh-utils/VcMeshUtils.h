/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include <array>
#include <cstdint>
#include <iosfwd>
#include <optional>
#include <utility>
#include <string>
#include <vector>

namespace SwApi
{
namespace VcMeshUtils
{

using namespace std;

using VcCpMesh = std::vector<uint32_t>;
using VcCpFloatMesh = std::vector<float>;
using VcStepMesh = std::vector<uint32_t>;
using VcStepSampleCoordMesh = std::vector<float>;

/// Reads meshes from the input `f`, first viewed as a bitstring, then attempting JSON.
/// Usage:
///   From just a string:
///     stringstream meshString{"{}"}
///     auto meshResult = ReadCpMeshesFrom(meshString);
///     bool parsedOk = meshResult.has_value();
///     vector<VcCpMesh> meshes = *meshResult;
///
///   From just a file:
///     ifstream meshFile{"./path/to/file.json"};
///     auto meshResult = ReadCpMeshesFrom(meshFile);
///     bool parsedOk = meshResult.has_value();
///     vector<VcCpMesh> meshes = *meshResult;
optional<vector<VcCpMesh>> ReadCpMeshesFrom(istream& f);

/// Reads a CP mesh, encoded as a string of 1s and 0s, from the input `f`.
/// Usage:
///   With just a string:
///     stringstream meshString; meshString << "0000001010011010010\n0000001001110011000";
///     auto meshResult = ReadCpMeshFromBin(meshString);
///     bool parsedOk = meshResult.has_value();
///     if (parsedOk) {
///       vector<VcCpMesh> meshes = *meshResult;
///     }
optional<VcCpMesh> ReadCpMeshFromBin(istream& f);

/// Reads a CP mesh, encoded in JSON format, from the input `f`.
optional<vector<VcCpMesh>> ReadCpMeshesFromJson(istream& f);
/// Reads a CP mesh (without quantizing it) encoded in JSON format from the input `f`.
optional<vector<VcCpFloatMesh>> ReadCpFloatMeshesFromJson(istream& f);

/// Given a single-color plane (intensity) mesh, split it into RGGB intensities.
array<VcCpFloatMesh, 4> SplitIntensityMeshIntoBayerChannels(const VcCpFloatMesh& intensities);

optional<VcStepMesh> ReadStepMeshFromBin(istream& f);

std::pair<VcStepMesh, VcStepSampleCoordMesh> GenerateStepMesh(uint16_t resX, uint16_t resY, uint8_t pip, uint16_t meshX, uint16_t meshY);

struct MeshGenParameters {
    float maxMeshIntensity = 2.0;
    float lensRadius = 2.0;
    float strength = 1.0;
};

float maxEntryIn(const VcCpFloatMesh& mesh);

template<uint32_t BIT_COUNT>
static constexpr uint32_t bitmask()
{
    return (1u << BIT_COUNT) - 1;
}


template<uint32_t I, uint32_t F>
static constexpr uint32_t ToFixedPoint(const float v)
{
    return static_cast<uint32_t>(v * static_cast<float>(1 << F) + 0.5f) & bitmask<I+F>();
}

VcCpMesh quantizeMeshToFixedPoint8i11f(const std::vector<float>& from);

VcCpFloatMesh MirrorMesh(const VcCpFloatMesh& mesh, float max = -1.0f);

VcCpMesh GenerateUnityMesh(unsigned int hMeshPoints, unsigned int vMeshPoints, int bitcnt);
VcCpFloatMesh GenerateUnityFMesh(unsigned int hMeshPoints,
                                 unsigned int vMeshPoints);

VcCpFloatMesh GenerateFloatMeshFor(unsigned int hMeshPoints,
                                   unsigned int vMeshPoints,
                                   MeshGenParameters params);

VcCpMesh GenerateMeshFor(unsigned int hMeshPoints,
                         unsigned int vMeshPoints,
                         MeshGenParameters params);

VcCpFloatMesh GenerateMirroredFloatMeshFor(unsigned int hMeshPoints,
                                           unsigned int vMeshPoints,
                                           MeshGenParameters params);

VcCpMesh GenerateMirroredMeshFor(unsigned int hMeshPoints,
                                 unsigned int vMeshPoints,
                                 MeshGenParameters params);


/// Format the value x as a `bitcnt`-wide bit string.
std::string formatAsBinary(uint32_t x, unsigned int bitcnt = 32);

std::ostream& PrintInBinary(std::ostream& os, const VcCpMesh& mesh, const char* sep = "\n");
std::ostream& PrintAsTable(std::ostream& os, const VcCpMesh& mesh, unsigned int hMeshPoints);

std::ostream& operator<<(std::ostream& os, const MeshGenParameters& params);
std::string ToString(const MeshGenParameters& mesh);

std::ostream& operator<<(std::ostream& os, const VcCpMesh& mesh);
std::string ToString(const VcCpMesh& mesh);

} // namespace VcMeshUtils
} // namespace SwApi
