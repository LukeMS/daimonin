########################
# Objects
########################
Mesh filenames:

Filename => species+ "_" + gender + "_" + profession + ".mesh"
gender   => (M)ale , (F)emale , (N)eutrum
examples => Human_M_Fighter.mesh , Orc_F_Child.mesh , Daemon_N_Hellhound.mesh

Equipment bones:

Bone_Right_Hand: "RFingers"
Bone_Left_Hand : "LFingers"
Bone_Head      : "Head"
Bone_Body      : "Spline1"

########################
# Weapons / Armor
########################
Filename => category + "_" + subcategory + "_" [+Category if needed] + counter 
examples => Sword_Short_01.mesh , Dagger_Long_Magic01.mesh , Sword_Bi_Broken.mesh

Filename => category +".material"
(Only one material-file per category will be used)

# Material names:
Material => MeshName + "/" + texture-pos.
examples => Sword_Short_01/Blade , Staff_Long_01/bar

ALL is case sensitive!
