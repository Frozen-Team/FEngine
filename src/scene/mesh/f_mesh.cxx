#include <algorithm>

#include "f_mesh.hpp"
#include "components/f_logger.hpp"

namespace fengine {
	const FShared<FGeometry> FMesh::GetGeometry(float distance) const
	{
		LOG_IF(lods_.size() == 0, FATAL);
		auto& found = std::find_if(lods_.cbegin(), lods_.cend(), [distance](const FMeshLod& lod) { return lod.IsVisible(distance); });
		if (found != lods_.cend()) {
			return found->geometry();
		}
		return lods_.nth(lods_.size() - 1).get_ptr()->geometry();
	}

	void FMesh::AddLod(float threshold, FShared<FGeometry> geometry)
	{	
		LOG_IF(threshold < 0 || !geometry, FATAL);
		auto is_inserted = this->lods_.insert(FMeshLod(threshold, geometry));
		LOG_IF(!is_inserted.second, FATAL);
	}
}

