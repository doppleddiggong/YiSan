import unreal
import os

# 에디터 유틸리티 모듈
editor_util = unreal.EditorUtilityLibrary()
editor_asset = unreal.EditorAssetLibrary()
material_editing = unreal.MaterialEditingLibrary()
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()

class MaterialCreatorTool:
    def __init__(self):
        self.selected_mesh = None
        self.texture_paths = {
            'base_color': None,
            'metallic': None,
            'roughness': None,
            'normal': None,
            'emissive': None,
            'ambient_occlusion': None,
            'opacity': None
        }
    
    def search_static_mesh_by_name(self, search_name):
        """이름으로 Static Mesh를 검색합니다"""
        # 모든 Static Mesh 에셋 검색
        filter = unreal.ARFilter(
            class_names=["StaticMesh"],
            recursive_paths=True
        )
        
        all_assets = asset_registry.get_assets(filter)
        
        # 검색어와 매칭되는 에셋 찾기
        matching_meshes = []
        search_lower = search_name.lower()
        
        for asset_data in all_assets:
            asset_name = asset_data.asset_name.lower()
            if search_lower in asset_name:
                matching_meshes.append(asset_data)
        
        return matching_meshes
    
    def show_mesh_selection_dialog(self):
        """Static Mesh 검색 및 선택 다이얼로그"""
        # 사용자로부터 검색어 입력받기
        search_query = unreal.EditorDialog.show_message(
            title="Static Mesh 검색",
            message="검색할 Static Mesh 이름을 입력하세요:",
            message_type=unreal.AppMsgType.OK_CANCEL,
            default_value=""
        )
        
        if search_query == unreal.AppReturnType.CANCEL:
            unreal.log("취소되었습니다.")
            return False
        
        # 간단한 텍스트 입력 받기 (언리얼 5.x에서는 제한적)
        # 실제 구현에서는 slate UI를 사용하는 것이 좋습니다
        unreal.log("Static Mesh를 Content Browser에서 선택하고 실행해주세요.")
        unreal.log("또는 아래 함수에 메시 이름을 직접 입력하세요.")
        
        # Content Browser에서 선택된 에셋 가져오기
        selected_assets = editor_util.get_selected_assets()
        
        for asset in selected_assets:
            if isinstance(asset, unreal.StaticMesh):
                self.selected_mesh = asset
                unreal.log(f"✓ 선택된 Static Mesh: {asset.get_name()}")
                return True
        
        # 선택된 게 없으면 전체 검색
        unreal.log("\n선택된 Static Mesh가 없습니다. 프로젝트에서 검색합니다...")
        all_meshes = self.get_all_static_meshes()
        
        if all_meshes:
            unreal.log(f"\n프로젝트 내 Static Mesh 목록 ({len(all_meshes)}개):")
            for i, mesh_data in enumerate(all_meshes[:20]):  # 처음 20개만 표시
                unreal.log(f"  {i+1}. {mesh_data.asset_name}")
            
            if len(all_meshes) > 20:
                unreal.log(f"  ... 외 {len(all_meshes) - 20}개")
            
            # 첫 번째 메시 자동 선택 (테스트용)
            if len(all_meshes) > 0:
                self.selected_mesh = all_meshes[0].get_asset()
                unreal.log(f"\n✓ 자동 선택됨: {self.selected_mesh.get_name()}")
                return True
        
        unreal.log_warning("Static Mesh를 Content Browser에서 선택해주세요!")
        return False
    
    def get_all_static_meshes(self):
        """프로젝트의 모든 Static Mesh 가져오기"""
        filter = unreal.ARFilter(
            class_names=["StaticMesh"],
            recursive_paths=True
        )
        return asset_registry.get_assets(filter)
    
    def select_texture_file(self, texture_type):
        """텍스처 파일을 선택합니다"""
        selected_files = unreal.EditorDialog.open_file_dialog(
            title=f"Select {texture_type} Texture",
            default_path="",
            file_types="Image Files (*.png;*.jpg;*.jpeg;*.tga;*.exr;*.hdr)|*.png;*.jpg;*.jpeg;*.tga;*.exr;*.hdr"
        )
        
        if selected_files and len(selected_files) > 0:
            self.texture_paths[texture_type] = selected_files[0]
            unreal.log(f"✓ {texture_type} 선택됨: {os.path.basename(selected_files[0])}")
            return True
        else:
            unreal.log(f"⊘ {texture_type} 건너뜀")
        return False
    
    def import_texture(self, file_path, destination_path):
        """텍스처 파일을 언리얼 프로젝트로 임포트"""
        if not file_path or not os.path.exists(file_path):
            return None
        
        # 텍스처 이름 추출
        texture_name = os.path.splitext(os.path.basename(file_path))[0]
        
        # 이미 임포트된 텍스처가 있는지 확인
        existing_texture_path = f"{destination_path}/{texture_name}"
        if editor_asset.does_asset_exist(existing_texture_path):
            unreal.log(f"기존 텍스처 사용: {texture_name}")
            return editor_asset.load_asset(existing_texture_path)
        
        # 임포트 태스크 생성
        task = unreal.AssetImportTask()
        task.filename = file_path
        task.destination_path = destination_path
        task.destination_name = texture_name
        task.replace_existing = True
        task.automated = True
        task.save = True
        
        # 텍스처 팩토리 설정
        task.factory = unreal.TextureFactory()
        
        # 임포트 실행
        asset_tools.import_asset_tasks([task])
        
        # 임포트된 에셋 가져오기
        imported_path = f"{destination_path}/{texture_name}"
        if editor_asset.does_asset_exist(imported_path):
            return editor_asset.load_asset(imported_path)
        
        return None
    
    def create_material_with_textures(self):
        """텍스처들을 연결한 Material을 생성"""
        if not self.selected_mesh:
            unreal.log_error("Static Mesh가 선택되지 않았습니다!")
            return None
        
        # Material 생성 경로 설정
        mesh_path = self.selected_mesh.get_path_name()
        mesh_dir = os.path.dirname(mesh_path)
        material_name = f"M_{self.selected_mesh.get_name()}_Auto"
        
        # 기존 Material이 있으면 덮어쓰기
        material_path = f"{mesh_dir}/{material_name}"
        if editor_asset.does_asset_exist(material_path):
            unreal.log(f"기존 Material 덮어쓰기: {material_name}")
            material = editor_asset.load_asset(material_path)
        else:
            # 새 Material 생성
            material = asset_tools.create_asset(
                material_name,
                mesh_dir,
                unreal.Material,
                unreal.MaterialFactoryNew()
            )
        
        if not material:
            unreal.log_error("Material 생성 실패!")
            return None
        
        unreal.log(f"\n=== Material 생성: {material_name} ===")
        
        # 텍스처 저장 디렉토리
        texture_dir = f"{mesh_dir}/Textures"
        
        # 각 텍스처 임포트 및 연결
        texture_configs = [
            ('base_color', 'RGB', unreal.MaterialProperty.MP_BASE_COLOR, None),
            ('metallic', 'R', unreal.MaterialProperty.MP_METALLIC, None),
            ('roughness', 'R', unreal.MaterialProperty.MP_ROUGHNESS, None),
            ('normal', 'RGB', unreal.MaterialProperty.MP_NORMAL, 'normal'),
            ('emissive', 'RGB', unreal.MaterialProperty.MP_EMISSIVE_COLOR, None),
            ('ambient_occlusion', 'R', unreal.MaterialProperty.MP_AMBIENT_OCCLUSION, None),
            ('opacity', 'R', unreal.MaterialProperty.MP_OPACITY, 'opacity'),
        ]
        
        for tex_key, channel, mat_property, special_type in texture_configs:
            if not self.texture_paths[tex_key]:
                continue
            
            texture = self.import_texture(self.texture_paths[tex_key], texture_dir)
            if not texture:
                continue
            
            # Normal Map 특수 설정
            if special_type == 'normal':
                texture.set_editor_property('compression_settings', 
                    unreal.TextureCompressionSettings.TC_NORMALMAP)
                texture.set_editor_property('srgb', False)
                unreal.log(f"  ✓ Normal Map 설정 적용")
            
            # Opacity 사용 시 Blend Mode 변경
            if special_type == 'opacity':
                material.set_editor_property('blend_mode', 
                    unreal.BlendMode.BLEND_TRANSLUCENT)
                unreal.log(f"  ✓ Blend Mode → Translucent")
            
            # Material에 텍스처 연결
            material_editing.connect_material_property(
                texture,
                channel,
                mat_property
            )
            unreal.log(f"  ✓ {tex_key.replace('_', ' ').title()} 연결됨")
        
        # Material 저장
        editor_asset.save_asset(material.get_path_name())
        
        # Static Mesh에 Material 적용
        self.selected_mesh.set_material(0, material)
        editor_asset.save_asset(self.selected_mesh.get_path_name())
        
        unreal.log(f"\n✓✓✓ 완료! Material '{material_name}'이 '{self.selected_mesh.get_name()}'에 적용되었습니다! ✓✓✓\n")
        
        return material


def run_material_creator():
    """Material Creator 메인 실행 함수"""
    unreal.log("\n" + "="*60)
    unreal.log("    AUTO MATERIAL CREATOR - UE 5.6")
    unreal.log("="*60 + "\n")
    
    creator = MaterialCreatorTool()
    
    # 1. Static Mesh 선택
    if not creator.show_mesh_selection_dialog():
        unreal.log_warning("Static Mesh가 선택되지 않아 종료합니다.")
        return
    
    unreal.log(f"\n선택된 Mesh: {creator.selected_mesh.get_name()}")
    unreal.log("\n--- 텍스처 선택 단계 ---")
    unreal.log("각 텍스처를 선택하세요 (취소 = 건너뛰기)\n")
    
    # 2. 텍스처 선택
    texture_types = [
        ('base_color', 'Base Color (Diffuse/Albedo)'),
        ('metallic', 'Metallic'),
        ('roughness', 'Roughness'),
        ('normal', 'Normal Map'),
        ('emissive', 'Emissive'),
        ('ambient_occlusion', 'Ambient Occlusion (AO)'),
        ('opacity', 'Opacity/Alpha')
    ]
    
    for tex_key, tex_label in texture_types:
        creator.select_texture_file(tex_key)
    
    # 3. Material 생성
    unreal.log("\n--- Material 생성 중... ---\n")
    material = creator.create_material_with_textures()
    
    if not material:
        unreal.log_error("✗ Material 생성 실패!")
        return
    
    # 완료 메시지
    unreal.log("="*60)
    unreal.log("  Material 생성 완료!")
    unreal.log("="*60)


# ========================================
# 에디터 메뉴에 추가
# ========================================
def register_menu():
    """에디터 상단 메뉴바에 등록"""
    menus = unreal.ToolMenus.get()
    
    # LevelEditor 메뉴 가져오기
    main_menu = menus.find_menu("LevelEditor.MainMenu")
    if not main_menu:
        unreal.log_error("메인 메뉴를 찾을 수 없습니다.")
        return
    
    # 커스텀 메뉴 섹션 추가 또는 가져오기
    custom_menu = menus.find_menu("LevelEditor.MainMenu.CustomTools")
    if not custom_menu:
        custom_menu = main_menu.add_sub_menu(
            owner=main_menu.menu_name,
            section_name="",
            name="CustomTools",
            label="Custom Tools"
        )
    
    # Material Creator 메뉴 항목 추가
    entry = unreal.ToolMenuEntry(
        name="AutoMaterialCreator",
        type=unreal.MultiBlockType.MENU_ENTRY
    )
    entry.set_label("Auto Material Creator")
    entry.set_tool_tip("Static Mesh에 자동으로 Material을 생성하고 텍스처를 연결합니다")
    entry.set_string_command(
        type=unreal.ToolMenuStringCommandType.PYTHON,
        custom_type="",
        string="import auto_material_creator; auto_material_creator.run_material_creator()"
    )
    
    custom_menu.add_menu_entry("Items", entry)
    
    menus.refresh_all_widgets()
    unreal.log("✓ 'Custom Tools > Auto Material Creator' 메뉴가 추가되었습니다!")


# 스크립트 실행 시 메뉴 등록
if __name__ == "__main__":
    # 직접 실행 시
    run_material_creator()
else:
    # 모듈로 임포트 시 메뉴 등록
    register_menu()
