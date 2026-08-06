/*******************************************************************************
Copyright (C) Altera Corporation
 
This code and the related documents are Altera copyrighted materials and your
use of them is governed by the express license under which they were provided to
you ("License"). This code and the related documents are provided as is, with no
express or implied warranties other than those that are expressly stated in the
License.
*******************************************************************************/

#pragma once

#include <vector>
#include <filesystem>

namespace SwApi
{
	namespace Lut3d
	{
		class LutContainer
		{
		public:
			typedef std::vector<float> TableEntry;
			typedef std::vector<std::vector<std::vector<TableEntry>>> LUT3DData;

			LutContainer( );
			LutContainer( const std::filesystem::path cubeFile );

			bool LoadCubeFile( const std::filesystem::path cubeFile );
			bool SaveCubeFile( const std::filesystem::path cubeFile );

			size_t GetDimension();
			bool GetTableEntry( const size_t r_index, const size_t g_index, const size_t b_index, TableEntry & entry );

			bool Resize( const size_t newSize );
			void Reset( );

		private:
			TableEntry InterpolateIndex( const float r_index,
											const float g_index,
											const float b_index );
			
			TableEntry LERPTableEntry( TableEntry & a, TableEntry & b, float t );
            int32_t LimitRange(int32_t value);

			bool m_initialised;
			size_t m_size;
			LUT3DData m_lut_table;
		};

	} // namespace Lut3d
} // namespace SwApi