#include "includes.h"

// im not sure we need to do the shit with setting ShouldSkipAnimFrame
// since it seems like thats only called by SetupBones itself, and we are rebuilding it.
// but ill leave it, just incase.

Bones g_bones{};;

bool Bones::setup( Player* player, BoneArray* out, LagRecord* record )
{
	// if the record isnt setup yet.
	if ( !record->m_setup )
	{
		// run setupbones rebuilt.
		if ( !BuildBones( player, BONE_USED_BY_ANYTHING & ~BONE_USED_BY_BONE_MERGE, record->m_bones, record ) )
			return false;

		// we have setup this record bones.
		record->m_setup = true;
	}

	// record is setup.
	if ( out && record->m_setup )
		std::memcpy( out, record->m_bones, sizeof( BoneArray ) * 128 );

	return true;
}

bool Bones::BuildBones( Player* target, int mask, BoneArray* out, LagRecord* record )
{
	vec3_t		     pos[ 128 ];
	quaternion_t     q[ 128 ];
	vec3_t           backup_origin;
	ang_t            backup_angles;
	float            backup_poses[ 24 ];
	C_AnimationLayer backup_layers[ 13 ];

	// get hdr.
	CStudioHdr* hdr = target->GetModelPtr();
	if ( !hdr )
		return false;

	// get ptr to bone accessor.
	CBoneAccessor* accessor = &target->m_BoneAccessor();
	if ( !accessor )
		return false;

	// store origial output matrix.
	// likely cachedbonedata.
	BoneArray* backup_matrix = accessor->m_pBones;
	if ( !backup_matrix )
		return false;

	g_csgo.m_model_cache->BeginLock();

	// prevent the game from calling ShouldSkipAnimationFrame.
	auto bSkipAnimationFrame = *reinterpret_cast<int*>( uintptr_t( target ) + 0x260 );
	*reinterpret_cast<int*>( uintptr_t( target ) + 0x260 ) = NULL;

	// backup original.
	backup_origin = target->GetAbsOrigin();
	backup_angles = target->GetAbsAngles();
	target->GetPoseParameters( backup_poses );
	target->GetAnimLayers( backup_layers );

	// compute transform from raw data.
	matrix3x4_t transform;
	math::AngleMatrix( record->m_abs_ang, record->m_origin, transform );

	// set non interpolated data.
	target->AddEffect( EF_NOINTERP );
	target->SetAbsOrigin( record->m_origin );
	target->SetAbsAngles( record->m_abs_ang );
	target->SetPoseParameters( record->m_poses );
	target->SetAnimLayers( record->m_layers );

	// force game to call AccumulateLayers - pvs fix.
	m_running = true;

	// set bone array for write.
	accessor->m_pBones = out;

	// compute and build bones.
	target->StandardBlendingRules( hdr, pos, q, record->m_sim_time, mask );

	uint8_t computed[ 0x100 ];
	std::memset( computed, 0, 0x100 );
	target->BuildTransformations( hdr, pos, q, transform, mask, computed );

	// restore old matrix.
	accessor->m_pBones = backup_matrix;

	// restore original interpolated entity data.
	target->SetAbsOrigin( backup_origin );
	target->SetAbsAngles( backup_angles );
	target->SetPoseParameters( backup_poses );
	target->SetAnimLayers( backup_layers );

	// revert to old game behavior.
	m_running = false;

	// allow the game to call ShouldSkipAnimationFrame.
	*reinterpret_cast<int*>( uintptr_t( target ) + 0x260 ) = bSkipAnimationFrame;

	g_csgo.m_model_cache->EndLock();

	return true;
}

bool Bones::Bro( Player* target, int mask, BoneArray* out )
{
	vec3_t		     pos[ 128 ];
	quaternion_t     q[ 128 ];

	CStudioHdr* hdr = target->GetModelPtr();
	if ( !hdr )
		return false;

	CBoneAccessor* accessor = &target->m_BoneAccessor();
	if ( !accessor )
		return false;

	BoneArray* backup_matrix = accessor->m_pBones;
	if ( !backup_matrix )
		return false;

	// prevent the game from calling ShouldSkipAnimationFrame.
	auto bSkipAnimationFrame = *reinterpret_cast<int*>( uintptr_t( target ) + 0x260 );
	*reinterpret_cast<int*>( uintptr_t( target ) + 0x260 ) = NULL;

	g_csgo.m_model_cache->BeginLock();

	matrix3x4_t transform;
	math::AngleMatrix( target->GetAbsAngles(), target->GetAbsOrigin(), transform);

	m_running = true;

	accessor->m_pBones = out;

	target->StandardBlendingRules( hdr, pos, q, g_csgo.m_globals->m_curtime, mask );

	uint8_t computed[ 0x100 ];
	std::memset( computed, 0, 0x100 );
	target->BuildTransformations( hdr, pos, q, transform, mask, computed );

	accessor->m_pBones = backup_matrix;

	m_running = false;

	*reinterpret_cast<int*>( uintptr_t( target ) + 0x260 ) = bSkipAnimationFrame;

	g_csgo.m_model_cache->EndLock();

	return true;
}

bool Bones::update_interp( LagRecord* record )
{
	vec3_t		     pos[ 128 ];
	quaternion_t     q[ 128 ];
	vec3_t           backup_origin;
	ang_t            backup_angles;
	float            backup_poses[ 24 ];
	C_AnimationLayer backup_layers[ 13 ];

	CStudioHdr* hdr = record->m_player->GetModelPtr();
	if ( !hdr )
		return false;

	CBoneAccessor* accessor = &record->m_player->m_BoneAccessor();
	if ( !accessor )
		return false;

	BoneArray* backup_matrix = accessor->m_pBones;
	if ( !backup_matrix )
		return false;

	g_csgo.m_model_cache->BeginLock();

	auto bSkipAnimationFrame = *reinterpret_cast<int*>( uintptr_t( record->m_player ) + 0x260 );
	*reinterpret_cast<int*>( uintptr_t( record->m_player ) + 0x260 ) = NULL;

	backup_origin = record->m_player->GetAbsOrigin();
	backup_angles = record->m_player->GetAbsAngles();
	record->m_player->GetPoseParameters( backup_poses );
	record->m_player->GetAnimLayers( backup_layers );

	matrix3x4_t transform;
	math::AngleMatrix( record->m_interp_ang, record->m_interp_org, transform );

	//record->m_player->AddEffect( EF_NOINTERP );
	record->m_player->SetAbsOrigin( record->m_interp_org );
	record->m_player->SetAbsAngles( record->m_interp_ang );
	record->m_player->SetPoseParameters( record->m_poses );
	record->m_player->SetAnimLayers( record->m_layers );

	m_running = true;

	accessor->m_pBones = record->m_bones;

	//record->m_player->StandardBlendingRules( hdr, pos, q, g_csgo.m_globals->m_curtime, BONE_USED_BY_ANYTHING & ~BONE_USED_BY_BONE_MERGE );

	uint8_t computed[ 0x100 ];
	std::memset( computed, 0, 0x100 );
	record->m_player->BuildTransformations( hdr, pos, q, transform, BONE_USED_BY_ANYTHING & ~BONE_USED_BY_BONE_MERGE, computed );

	accessor->m_pBones = backup_matrix;

	record->m_player->SetAbsOrigin( backup_origin );
	record->m_player->SetAbsAngles( backup_angles );
	record->m_player->SetPoseParameters( backup_poses );
	record->m_player->SetAnimLayers( backup_layers );

	m_running = false;

	*reinterpret_cast<int*>( uintptr_t( record->m_player ) + 0x260 ) = bSkipAnimationFrame;

	g_csgo.m_model_cache->EndLock();

	return true;
}