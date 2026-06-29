import java.io.File
import java.util.Properties

plugins {
    alias(libs.plugins.android.application)
    alias(libs.plugins.kotlin.compose)
}

android {
    namespace = "com.example.starquant"
    compileSdk {
        version = release(36) {
            minorApiLevel = 1
        }
    }

    defaultConfig {
        applicationId = "com.example.starquant"
        minSdk = 24
        targetSdk = 36
        versionCode = 1
        versionName = "1.0"

        testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
    }

    buildTypes {
        release {
            optimization {
                enable = false
            }
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_11
        targetCompatibility = JavaVersion.VERSION_11
    }
    buildFeatures {
        compose = true
    }
}

dependencies {
    implementation(platform(libs.androidx.compose.bom))
    implementation(libs.androidx.activity.compose)
    implementation(libs.androidx.compose.material3)
    implementation(libs.androidx.compose.material.icons.extended)
    implementation(libs.androidx.compose.ui)
    implementation(libs.androidx.compose.ui.graphics)
    implementation(libs.androidx.compose.ui.tooling.preview)
    implementation(libs.androidx.core.ktx)
    implementation(libs.androidx.lifecycle.runtime.ktx)
    implementation(libs.androidx.lifecycle.viewmodel.compose)
    implementation(libs.androidx.navigation.compose)
    implementation(libs.kotlinx.coroutines.android)
    implementation(libs.kotlinx.coroutines.core)
    implementation(libs.okhttp)
    implementation(libs.gson)
    implementation(libs.coil.compose)
    testImplementation(libs.junit)
    androidTestImplementation(platform(libs.androidx.compose.bom))
    androidTestImplementation(libs.androidx.compose.ui.test.junit4)
    androidTestImplementation(libs.androidx.espresso.core)
    androidTestImplementation(libs.androidx.junit)
    debugImplementation(libs.androidx.compose.ui.test.manifest)
    debugImplementation(libs.androidx.compose.ui.tooling)
}

val normalizeDebugApkIdeRedirect by tasks.registering {
    group = "build"
    description = "Writes stable Android Studio debug APK metadata after IDE builds."

    doLast {
        val redirectFile = layout.buildDirectory
            .file("intermediates/apk_ide_redirect_file/debug/createDebugApkListingFileRedirect/redirect.txt")
            .get()
            .asFile
        val metadataFile = listOf(
            layout.buildDirectory.file("outputs/apk/debug/output-metadata.json").get().asFile,
            layout.buildDirectory.file("intermediates/apk/debug/output-metadata.json").get().asFile,
        ).filter { it.isFile }
            .maxByOrNull { it.lastModified() }
            ?: return@doLast
        val apkFile = metadataFile.resolveSibling("app-debug.apk")
        if (!apkFile.isFile) {
            return@doLast
        }

        redirectFile.parentFile.mkdirs()
        val normalizedText = metadataFile.readText(Charsets.UTF_8)
            .replace(
                Regex(""""outputFile"\s*:\s*"[^"]+""""),
                """"outputFile": "${apkFile.invariantSeparatorsPath}""""
            )
        val tempFile = redirectFile.resolveSibling("${redirectFile.name}.tmp")
        tempFile.writeText(normalizedText, Charsets.UTF_8)
        if (redirectFile.exists()) {
            redirectFile.delete()
        }
        tempFile.renameTo(redirectFile)
        Thread.sleep(3_000)
    }
}

tasks.matching {
    it.name == "createDebugApkListingFileRedirect" || it.name == "assembleDebug"
}.configureEach {
    finalizedBy(normalizeDebugApkIdeRedirect)
}

val localSdkDir = Properties().let { props ->
    val localPropertiesFile = rootProject.file("local.properties")
    if (localPropertiesFile.isFile) {
        localPropertiesFile.inputStream().use { props.load(it) }
    }
    props.getProperty("sdk.dir")
}

tasks.register<Exec>("installAndStartDebug") {
    group = "install"
    description = "Installs the debug APK and starts the app without Android Studio APK metadata parsing."
    dependsOn("installDebug")

    val sdkDir = providers.environmentVariable("ANDROID_HOME")
        .orElse(providers.environmentVariable("ANDROID_SDK_ROOT"))
        .orElse(localSdkDir ?: "D:/AndridSdk")
        .get()
    commandLine(
        File(sdkDir, "platform-tools/adb.exe").absolutePath,
        "shell",
        "am",
        "start",
        "-n",
        "com.example.starquant/.MainActivity",
    )
}
