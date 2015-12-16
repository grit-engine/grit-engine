#!/bin/bash

(cd ~/gritengine/grit_core/linux && make gsl.linux.x86_64) || exit 1


TMP=$(tempfile)

do_glsl_check() {
    KIND="$1"
    FILENAME="$2.glsl"
    ./glsl_check "${KIND}" "${FILENAME}"
    CODE="$?"
    return "$CODE"
}

do_cg_check() {
    KIND="$1"
    FILENAME="$2.cg"
    PROFILE=ps_3_0
    if [ "$KIND" == "vert" ] ; then
        PROFILE=vs_3_0
    fi
    cgc -profile $PROFILE -strict ${FILENAME} -o ${FILENAME}.asm
    CODE="$?"
    return "$CODE"
}

do_check() {
    TARGET="$1"
    KIND="$2"
    FILENAME="$3"
    if [ "$TARGET" == "cg" ] ; then
        do_cg_check $KIND $FILENAME > $TMP 2>&1
    else
        do_glsl_check $KIND $FILENAME > $TMP 2>&1
    fi
    CODE="$?"
    if [ "$CODE" -ne 0 ] ; then
        echo "While checking ${TARGET} ${KIND} shader: ${FILENAME}"
        cat $TMP
        nl -b a ${FILENAME}
    fi
    return "$CODE"
}

test_sky() {
    TARGET="$1"
    SHADER="$2"
    PARAMS="-p starfieldMap FloatTexture2 -p starfieldMask Float3 -p perlin FloatTexture2 -p perlinN FloatTexture2 -p emissiveMap FloatTexture2 -p emissiveMask Float3 -p alphaMask Float -p alphaRejectThreshold Float"
    UBT="-u perlinN"
    TLANG=""
    test $TARGET == "cg" && TLANG="-C"
    gsl $TLANG $PARAMS $UBT "${SHADER}.vert.gsl" "" "${SHADER}.colour.gsl" SKY ${SHADER}.{vert,frag}.out.$TARGET || exit 1

    do_check ${TARGET} vert ${SHADER}.vert.out && do_check ${TARGET} frag ${SHADER}.frag.out
}




test_hud() {
    TARGET="$1"
    SHADER="$2"
    PARAMS="-p colour Float3 -p alpha Float -p tex FloatTexture2"
    UBT=""
    TLANG=""
    test $TARGET == "cg" && TLANG="-C"
    gsl $TLANG $PARAMS $UBT "${SHADER}.vert.gsl" "" "${SHADER}.colour.gsl" HUD ${SHADER}.{vert,frag}.out.$TARGET || exit 1

    do_check ${TARGET} vert ${SHADER}.vert.out && do_check ${TARGET} frag ${SHADER}.frag.out
}

# TODO(dcunnin): Need to test with -e and -E not to mention -d
test_first_person() {
    TARGET="$1"
    SHADER="$2"
    BONE_WEIGHTS="$3"
    PARAMS="-p alphaMask Float -p alphaRejectThreshold Float -p diffuseMap FloatTexture2 -p diffuseMask Float3 -p normalMap FloatTexture2 -p glossMap FloatTexture2 -p gloss Float -p specular Float -p emissiveMap FloatTexture2 -p emissiveMask Float3 -b $BONE_WEIGHTS"
    UBT="-u normalMap"
    TLANG=""
    test $TARGET == "cg" && TLANG="-C"
    gsl $TLANG $PARAMS $UBT "${SHADER}.vert.gsl" "${SHADER}.dangs.gsl" "${SHADER}.add.gsl" FIRST_PERSON ${SHADER}.${BONE_WEIGHTS}.{vert,frag}.out.$TARGET || exit 1

    do_check ${TARGET} vert ${SHADER}.${BONE_WEIGHTS}.vert.out && do_check ${TARGET} frag ${SHADER}.${BONE_WEIGHTS}.frag.out
}


# TODO(dcunnin): Need to test with -e and -E not to mention -d
test_decal() {
    TARGET="$1"
    SHADER="$2"
    BONE_WEIGHTS="$3"
    PARAMS="-p alphaMask Float -p alphaRejectThreshold Float -p diffuseMap FloatTexture2 -p diffuseMask Float3 -p normalMap FloatTexture2 -p glossMap FloatTexture2 -p gloss Float -p specular Float -p emissiveMap FloatTexture2 -p emissiveMask Float3 -b $BONE_WEIGHTS"
    UBT="-u normalMap"
    TLANG=""
    test $TARGET == "cg" && TLANG="-C"
    gsl $TLANG $PARAMS $UBT "" "${SHADER}.dangs.gsl" "${SHADER}.add.gsl" DECAL ${SHADER}.${BONE_WEIGHTS}.{vert,frag}.out.$TARGET || exit 1

    do_check ${TARGET} vert ${SHADER}.${BONE_WEIGHTS}.vert.out && do_check ${TARGET} frag ${SHADER}.${BONE_WEIGHTS}.frag.out
}


test_particle() {
    TARGET="$1"
    SHADER="$2"
    PARAMS="-p gbuffer0 FloatTexture2 -p particleAtlas FloatTexture2"
    TLANG=""
    test $TARGET == "cg" && TLANG="-C"
    gsl $TLANG $PARAMS $UBT "${SHADER}.vert.gsl" "" "${SHADER}.add.gsl" SKY ${SHADER}.{vert,frag}.out.$TARGET || exit 1

    do_check ${TARGET} vert ${SHADER}.vert.out && do_check ${TARGET} frag ${SHADER}.frag.out
}


do_tests() {
    TARGET=$1
    test_particle ${TARGET} Particle &&

    test_sky ${TARGET} Empty &&
    test_sky ${TARGET} SkyTest &&
    test_sky ${TARGET} SkyDefault &&
    test_sky ${TARGET} SkyClouds &&
    test_sky ${TARGET} SkyBackground &&

    test_first_person ${TARGET} FpDefault 0 &&
    test_first_person ${TARGET} Empty 0 &&
    test_first_person ${TARGET} FpDefault 3 &&
    test_first_person ${TARGET} Empty 3 &&

    test_hud ${TARGET} HudRect &&
    test_hud ${TARGET} HudText &&

    true
    return "$?"
}

if [ "$SKIP_GLSL" != "1" ] ; then
    do_tests glsl
fi

if [ "$SKIP_CG" != "1" ] ; then
    do_tests cg
fi

test_decal glsl Empty 0
