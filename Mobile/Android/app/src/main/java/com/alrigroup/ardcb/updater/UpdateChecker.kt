package com.alrigroup.ardcb.updater

import android.content.Context
import org.json.JSONObject
import java.io.BufferedReader
import java.io.InputStreamReader
import java.net.HttpURLConnection
import java.net.URL
import kotlin.concurrent.thread

object UpdateChecker {
    private const val GITHUB_REPO = "alrigroup/ARDiscordBypass"
    private const val API_URL = "https://api.github.com/repos/$GITHUB_REPO/releases/latest"
    const val CURRENT_VERSION = "3.0.0"

    data class UpdateInfo(
        val hasUpdate: Boolean,
        val latestVersion: String,
        val releaseNotes: String,
        val downloadUrl: String
    )

    fun check(onResult: (UpdateInfo) -> Unit) {
        thread {
            try {
                val url = URL(API_URL)
                val conn = url.openConnection() as HttpURLConnection
                conn.connectTimeout = 8000
                conn.readTimeout = 8000
                conn.setRequestProperty("User-Agent", "ARDiscordBypass-Android/$CURRENT_VERSION")
                conn.setRequestProperty("Accept", "application/vnd.github.v3+json")

                if (conn.responseCode == 200) {
                    val reader = BufferedReader(InputStreamReader(conn.inputStream))
                    val response = reader.readText()
                    reader.close()

                    val json = JSONObject(response)
                    var tagName = json.optString("tag_name", "")
                    if (tagName.startsWith("v", ignoreCase = true)) {
                        tagName = tagName.substring(1)
                    }

                    val body = json.optString("body", "Melhorias de estabilidade e novas rotas.")
                    var apkUrl = "https://github.com/$GITHUB_REPO/releases/latest"

                    val assets = json.optJSONArray("assets")
                    if (assets != null) {
                        for (i in 0 until assets.length()) {
                            val asset = assets.getJSONObject(i)
                            val name = asset.optString("name", "")
                            if (name.endsWith(".apk", ignoreCase = true)) {
                                apkUrl = asset.optString("browser_download_url", apkUrl)
                                break
                            }
                        }
                    }

                    val hasUpdate = isNewerVersion(tagName, CURRENT_VERSION)
                    onResult(UpdateInfo(hasUpdate, tagName, body, apkUrl))
                } else {
                    onResult(UpdateInfo(false, CURRENT_VERSION, "", ""))
                }
            } catch (_: Exception) {
                onResult(UpdateInfo(false, CURRENT_VERSION, "", ""))
            }
        }
    }

    private fun isNewerVersion(remote: String, current: String): Boolean {
        if (remote.isBlank() || remote == current) return false
        val rParts = remote.split(".").mapNotNull { it.toIntOrNull() }
        val cParts = current.split(".").mapNotNull { it.toIntOrNull() }
        val maxLen = maxOf(rParts.size, cParts.size)
        for (i in 0 until maxLen) {
            val r = rParts.getOrElse(i) { 0 }
            val c = cParts.getOrElse(i) { 0 }
            if (r > c) return true
            if (r < c) return false
        }
        return false
    }
}
