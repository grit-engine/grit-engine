#!/bin/bash

set -e

make -C ../../.. gsl

TMP=$(tempfile)

do_glsl_check() {
    local KIND="$1"
    local FILENAME="$2"
    ./glsl_check "${KIND}" "${FILENAME}" 2>&1
    return "$?"
}

do_cg_check() {
    local KIND="$1"
    local FILENAME="$2"
    local PROFILE=ps_3_0
    if [ "$KIND" == "vert" ] ; then
        PROFILE=vs_3_0
    fi
    cgc -profile $PROFILE -strict ${FILENAME} -o ${FILENAME}.asm 2>&1
    return "$?"
}

do_check() {
    local TARGET="$1"
    local KIND="$2"
    local FILENAME="$3"
    local FAILED=0
    if [ "$TARGET" == "cg" ] ; then
        do_cg_check $KIND $FILENAME > $TMP || FAILED=1
    else
        do_glsl_check $KIND $FILENAME > $TMP || FAILED=1
    fi
    if [ "$FAILED" -ne 0 ] ; then
        echo "While checking ${TARGET} ${KIND} shader: ${FILENAME}"
        cat $TMP
        nl -b a ${FILENAME}
        exit 1
    fi
}

test_sky() {
    local TARGET="$1"
    local SHADER="$2"
    local PARAMS="-p starfieldMap FloatTexture2 -p starfieldMask Float3 -p perlin FloatTexture2 -p perlinN FloatTexture2 -p emissiveMap FloatTexture2 -p emissiveMask Float3 -p alphaMask Float -p alphaRejectThreshold Float -p premultipliedAlpha StaticFloat"
    local UBT="-u perlinN"
    local TLANG=""
    test $TARGET == "cg" && TLANG="-C"
    gsl $TLANG $PARAMS $UBT "${SHADER}.vert.gsl" "/dev/null" "${SHADER}.colour.gsl" SKY ${SHADER}.{vert,frag}.out.$TARGET
    do_check ${TARGET} vert ${SHADER}.vert.out.${TARGET}
    do_check ${TARGET} frag ${SHADER}.frag.out.${TARGET}
}




test_hud() {
    local TARGET="$1"
    local SHADER="$2"
    local PARAMS="-p colour Float3 -p alpha Float -p tex FloatTexture2"
    local UBT=""
    local TLANG=""
    test $TARGET == "cg" && TLANG="-C"
    gsl $TLANG $PARAMS $UBT "${SHADER}.vert.gsl" "/dev/null" "${SHADER}.colour.gsl" HUD ${SHADER}.{vert,frag}.out.$TARGET
    do_check ${TARGET} vert ${SHADER}.vert.out.${TARGET}
    do_check ${TARGET} frag ${SHADER}.frag.out.${TARGET}
}

# TODO(dcunnin): Need to test with -e and -E not to mention -d
test_body() {
    local TARGET="$1"
    local SHADER="$2"
    local BONE_WEIGHTS="$3"
    local KIND="$4"
    local INSTANCED="$5"
    local PARAMS="$INSTANCED -p alphaMask Float -p alphaRejectThreshold Float -p diffuseMap FloatTexture2 -p diffuseMask Float3 -p normalMap FloatTexture2 -p glossMap FloatTexture2 -p glossMask Float -p specularMask Float -p emissiveMap FloatTexture2 -p emissiveMask Float3 -b $BONE_WEIGHTS"
    local UBT="-u normalMap"
    local TLANG=""
    test $TARGET == "cg" && TLANG="-C"
    gsl $TLANG $PARAMS $UBT "${SHADER}.vert.gsl" "${SHADER}.dangs.gsl" "${SHADER}.add.gsl" ${KIND} ${SHADER}.${BONE_WEIGHTS}.{vert,frag}.$KIND.out.$TARGET

    do_check ${TARGET} vert ${SHADER}.${BONE_WEIGHTS}.vert.$KIND.out.$TARGET
    do_check ${TARGET} frag ${SHADER}.${BONE_WEIGHTS}.frag.$KIND.out.$TARGET
}

# TODO(dcunnin): Need to test with -e and -E not to mention -d
test_decal() {
    local TARGET="$1"
    local SHADER="$2"
    local BONE_WEIGHTS="$3"
    local PARAMS="-p alphaMask Float -p alphaRejectThreshold Float -p diffuseMap FloatTexture2 -p diffuseMask Float3 -p normalMap FloatTexture2 -p glossMap FloatTexture2 -p glossMask Float -p specularMask Float -p emissiveMap FloatTexture2 -p emissiveMask Float3 -b $BONE_WEIGHTS"
    local UBT="-u normalMap"
    local TLANG=""
    test $TARGET == "cg" && TLANG="-C"
    gsl $TLANG $PARAMS $UBT "/dev/null" "${SHADER}.dangs.gsl" "${SHADER}.add.gsl" DECAL ${SHADER}.${BONE_WEIGHTS}.{vert,frag}.out.$TARGET
    do_check ${TARGET} vert ${SHADER}.${BONE_WEIGHTS}.vert.out.$TARGET
    do_check ${TARGET} frag ${SHADER}.${BONE_WEIGHTS}.frag.out.$TARGET
}


test_particle() {
    local TARGET="$1"
    local SHADER="$2"
    local PARAMS="--internal -p gbuffer0 FloatTexture2 -p particleAtlas FloatTexture2"
    local TLANG=""
    test $TARGET == "cg" && TLANG="-C"
    gsl $TLANG $PARAMS $UBT "${SHADER}.vert.gsl" "/dev/null" "${SHADER}.add.gsl" SKY ${SHADER}.{vert,frag}.out.$TARGET
    do_check ${TARGET} vert ${SHADER}.vert.out.$TARGET
    do_check ${TARGET} frag ${SHADER}.frag.out.$TARGET
}


do_tests() {
    local TARGET=$1
    test_particle ${TARGET} Particle

    test_sky ${TARGET} Empty
    test_sky ${TARGET} SkyTest
    test_sky ${TARGET} SkyDefault
    test_sky ${TARGET} SkyClouds
    test_sky ${TARGET} SkyBackground
    test_sky ${TARGET} ForLoop

    for KIND in FORWARD ALPHA FIRST_PERSON FIRST_PERSON_WIREFRAME CAST ; do
        for INSTANCED in "" "-i"; do
            test_body ${TARGET} FpDefault 0 "$KIND" "$INSTANCED"
            test_body ${TARGET} Empty 0 "$KIND" "$INSTANCED"
            test_body ${TARGET} CarPaint 0 "$KIND" "$INSTANCED -p paintSelectionMap FloatTexture2 -p paintSelectionMask Float4 -p paintByDiffuseAlpha StaticFloat -p microflakesMap FloatTexture2"
            test_body ${TARGET} FpDefault 3 "$KIND" "$INSTANCED"
            test_body ${TARGET} Empty 3 "$KIND" "$INSTANCED"
            test_body ${TARGET} CarPaint 3 "$KIND" "$INSTANCED -p paintSelectionMap FloatTexture2 -p paintSelectionMask Float4 -p paintByDiffuseAlpha StaticFloat -p microflakesMap FloatTexture2"
        done
    done

    test_hud ${TARGET} HudRect
    test_hud ${TARGET} HudText
}

if [ "$SKIP_GLSL" != "1" ] ; then
    do_tests glsl
fi

if [ "$SKIP_CG" != "1" ] ; then
    do_tests cg
fi

# test_decal glsl Empty 0
