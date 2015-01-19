#!/bin/bash

make -C ~/gritengine/grit_core/linux gsl.linux.x86_64 || exit 1



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
    PROFILE=hlslf
    if [ "$KIND" == "vert" ] ; then
        PROFILE=hlslv
    fi
    cgc -nocode -profile $PROFILE -strict ${FILENAME}
    CODE="$?"
    return "$CODE"
}

do_check() {
    TARGET="$1"
    KIND="$2"
    FILENAME="$3"
    echo "Checking ${TARGET} ${KIND} shader: ${FILENAME}"
    if [ "$TARGET" == "cg" ] ; then
        do_cg_check $KIND $FILENAME
    else
        do_glsl_check $KIND $FILENAME
    fi
    CODE="$?"
    if [ "$CODE" -ne 0 ] ; then
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

test_first_person() {
    TARGET="$1"
    SHADER="$2"
    PARAMS="-p alphaMask Float -p alphaRejectThreshold Float -p diffuseMap FloatTexture2 -p diffuseMask Float3 -p normalMap FloatTexture2 -p glossMap FloatTexture2 -p gloss Float -p specular Float -p emissiveMap FloatTexture2 -p emissiveMask Float3"
    UBT="-u normalMap"
    TLANG=""
    test $TARGET == "cg" && TLANG="-C"
    gsl $TLANG $PARAMS $UBT "${SHADER}.vert.gsl" "${SHADER}.dangs.gsl" "${SHADER}.add.gsl" FIRST_PERSON ${SHADER}.{vert,frag}.out.$TARGET || exit 1

    do_check ${TARGET} vert ${SHADER}.vert.out && do_check ${TARGET} frag ${SHADER}.frag.out
}


do_tests() {
    TARGET=$1
    test_sky ${TARGET} SkyEmpty &&
    test_sky ${TARGET} SkyTest &&
    test_sky ${TARGET} SkyDefault &&
    test_sky ${TARGET} SkyClouds &&
    test_sky ${TARGET} SkyBackground &&

    test_first_person ${TARGET} FpDefault &&
    test_first_person ${TARGET} FpEmpty &&

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
