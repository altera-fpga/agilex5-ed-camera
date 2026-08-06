/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#include "LutContainer.h"
#include <fstream>
#include <iomanip>
#include <limits>
#include <cmath>

namespace SwApi
{
    namespace Lut3d
    {

        LutContainer::LutContainer( )
        : m_initialised{ false }
        , m_size{ 0 }
        {

        }

        LutContainer::LutContainer( const std::filesystem::path cubeFile )
        : m_initialised{ false }
        , m_size{ 0 }
        {
            LoadCubeFile( std::move(cubeFile) );
        }

        bool LutContainer::LoadCubeFile( const std::filesystem::path cubeFile )
        {
            size_t size = 0;
            size_t linePos = 0;

            std::ifstream fileStream( cubeFile );

            if(!fileStream.good())
                return false;

            std::string line;

            //Parse metadata
            while( std::getline( fileStream, line ) )
            {
                std::istringstream lineStream( line );
                std::string firstWord;
                lineStream >> firstWord;

                if( "+" < firstWord && firstWord < ":" )
                {
                    //We have found a table entry without parsing the size tag yet
                    if(!size)
                    {
                        return false;
                    }
                    else
                    {
                        //Reset back to start of entry
                        fileStream.seekg( linePos );
                        break;
                    }
                }
                else if( firstWord == "LUT_3D_SIZE" )
                {
                    lineStream >> size;
                }

                linePos = fileStream.tellg( );
            }

            LUT3DData newTable( size, std::vector<std::vector<TableEntry>>(
                                size, std::vector<TableEntry>(
                                size, TableEntry(3, std::numeric_limits<float>::quiet_NaN()) ) ) );

            //Parse table
            for( size_t b = 0; b < size; b++ )
            {
                for( size_t g = 0; g < size; g++ )
                {
                    for( size_t r = 0; r < size; r++ )
                    {
                        while( std::getline( fileStream, line ) )
                        {
                            if(!line.empty() && '#' != line[0])
                            {
                                std::istringstream lineStream( line );
                                for(auto & value : newTable[b][g][r])
                                {
                                    lineStream >> value;
                                    if(std::isnan(value) || value > 1.0f || value < 0.0f)
                                        return false;
                                }

                                break;
                            }

                        }
                    }
                }
            }

            for( size_t b = 0; b < size; b++ )
                for( size_t g = 0; g < size; g++ )
                    for( size_t r = 0; r < size; r++ )
                        for(auto & value : newTable[b][g][r])
                            if(std::isnan(value))
                                return false;


            m_initialised = true;
            m_size = size;
            m_lut_table = std::move(newTable);

            return true;
        }

        bool LutContainer::SaveCubeFile( const std::filesystem::path cubeFile )
        {
            if(!m_initialised)
                return false;

            std::ofstream fileStream( cubeFile );

            if(!fileStream.good())
                return false;

            fileStream << "LUT_3D_SIZE " << m_size << std::endl;

            for( size_t b = 0; b < m_size; b++ )
            {
                for( size_t g = 0; g < m_size; g++ )
                {
                    for( size_t r = 0; r < m_size; r++ )
                    {
                        fileStream << std::setprecision(6) << std::fixed
                                << m_lut_table[b][g][r][0] << ' ' 
                                << m_lut_table[b][g][r][1] << ' ' 
                                << m_lut_table[b][g][r][2] << std::endl;	
                    }
                }
            }

            fileStream.flush();
            return fileStream.good();
        }

        bool LutContainer::Resize( const size_t newSize )
        {
            if( !m_initialised || newSize < 2 || newSize > 65)
                return false;

            if (m_size == newSize)
                return true;

            LUT3DData newTable( newSize, std::vector<std::vector<TableEntry>>(
                                newSize, std::vector<TableEntry>(
                                newSize ) ) );

            float ratio = static_cast<float>(m_size - 1) / static_cast<float>(newSize - 1);

            for( size_t b = 0; b < newSize; b++ )
            {
                for( size_t g = 0; g < newSize; g++ )
                {
                    for( size_t r = 0; r < newSize; r++ )
                    {
                        newTable[b][g][r] = InterpolateIndex( static_cast<float>(r) * ratio,
                                                            static_cast<float>(g) * ratio,
                                                            static_cast<float>(b) * ratio );
                    }
                }
            }

            m_size = newSize;
            m_lut_table = std::move(newTable);

            return true;
        }

        size_t LutContainer::GetDimension()
        {
            return m_size;
        }

        bool LutContainer::GetTableEntry( const size_t r_index, const size_t g_index, const size_t b_index, TableEntry & entry )
        {
            if ((r_index < m_size) && (g_index < m_size) && (b_index < m_size))
            {
                entry = m_lut_table[b_index][g_index][r_index];
                return true;
            }
            else
            {
                return false;
            }
        }

        void LutContainer::Reset()
        {
            m_lut_table.clear();
            m_size = 0;
            m_initialised = false;
        }
 
        int32_t LutContainer::LimitRange(int32_t value)
        {
            if (m_size == 0)
                return 0;
            else if (value < 0)
                return 0;
            else if (value > (static_cast<int32_t>(m_size) - 1))
                return static_cast<int32_t>(m_size) - 1;
            else
                return value;
        }

        LutContainer::TableEntry LutContainer::InterpolateIndex(const float r_index,
                                                                const float g_index,
                                                                const float b_index)
        {
            int32_t lowerR = LimitRange(static_cast<int32_t>(r_index));
            int32_t lowerG = LimitRange(static_cast<int32_t>(g_index));
            int32_t lowerB = LimitRange(static_cast<int32_t>(b_index));
            int32_t upperR = LimitRange(lowerR + 1);
            int32_t upperG = LimitRange(lowerG + 1);
            int32_t upperB = LimitRange(lowerB + 1);

            TableEntry CLLL = m_lut_table[lowerB][lowerG][lowerR]; 
            TableEntry CLHL = m_lut_table[upperB][lowerG][lowerR];
            TableEntry CHLL = m_lut_table[lowerB][lowerG][upperR];
            TableEntry CLLH = m_lut_table[lowerB][upperG][lowerR];
            TableEntry CHHL = m_lut_table[upperB][lowerG][upperR]; 
            TableEntry CHHH = m_lut_table[upperB][upperG][upperR]; 
            TableEntry CHLH = m_lut_table[lowerB][upperG][upperR];
            TableEntry CLHH = m_lut_table[upperB][upperG][lowerR]; 

            float dR = r_index - static_cast<float>(lowerR);
            float dB = b_index - static_cast<float>(lowerB);
            float dG = g_index - static_cast<float>(lowerG);

            TableEntry CLL = LERPTableEntry(CLLL, CHLL, dR);
            TableEntry CHL = LERPTableEntry(CLHL, CHHL, dR);
            TableEntry CLH = LERPTableEntry(CLLH, CHLH, dR);
            TableEntry CHH = LERPTableEntry(CLHH, CHHH, dR);

            TableEntry CH = LERPTableEntry(CLH, CHH, dB);
            TableEntry CL = LERPTableEntry(CLL, CHL, dB);

            return LERPTableEntry(CL, CH, dG);
        }

        LutContainer::TableEntry LutContainer::LERPTableEntry( TableEntry & a, TableEntry & b, float t )
        {
            TableEntry lerpedEntry(3);

            for(size_t index = 0; index < a.size(); index++)
            {
                lerpedEntry[index] = a[index] + ( b[index] - a[index] ) * t;
            }

            return lerpedEntry;
        }

    } // namespace Lut3d
} // namespace SwApi