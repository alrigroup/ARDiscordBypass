package com.alrigroup.ardcb.tunnel

import android.content.Context
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.withContext
import java.io.File

object ProtonAuthManager {

    fun getNativeBinary(context: Context): File? {
        val nativeDir = File(context.applicationInfo.nativeLibraryDir)
        val bin = File(nativeDir, "libprotonconfgen.so")
        return if (bin.exists() && bin.canExecute()) bin else null
    }

    fun hasSavedSession(context: Context): Boolean {
        val sessionFile = File(context.filesDir, ".protonvpn-session.json")
        return sessionFile.exists() && sessionFile.length() > 0
    }

    fun clearSession(context: Context) {
        val sessionFile = File(context.filesDir, ".protonvpn-session.json")
        if (sessionFile.exists()) sessionFile.delete()
    }

    suspend fun authenticateAndGenerate(
        context: Context,
        user: String,
        pass: String,
        onProgress: (String) -> Unit
    ): Result<File> = withContext(Dispatchers.IO) {
        val bin = getNativeBinary(context)
            ?: return@withContext Result.failure(IllegalStateException("Módulo nativo libprotonconfgen não encontrado."))

        val outputFile = File(context.filesDir, "ardc_vpn.conf")
        onProgress("Autenticando via SRP na API ProtonVPN...")

        val cmd = mutableListOf(
            bin.absolutePath,
            "--username", user.trim(),
            "--password", pass.trim(),
            "--free-only",
            "--countries", "US,NL,JP",
            "--output", outputFile.absolutePath
        )

        try {
            val pb = ProcessBuilder(cmd)
            pb.environment()["HOME"] = context.filesDir.absolutePath
            pb.redirectErrorStream(true)

            val process = pb.start()
            val outputBuilder = StringBuilder()
            val reader = process.inputStream.bufferedReader()
            var line = reader.readLine()
            while (line != null) {
                outputBuilder.appendLine(line)
                val trimmed = line.trim()
                when {
                    trimmed.contains("Authentication successful", ignoreCase = true) ->
                        onProgress("Credenciais aceitas! Consultando servidores...")
                    trimmed.contains("Selected server:", ignoreCase = true) ->
                        onProgress("Servidor selecionado: " + trimmed.substringAfter("Selected server: ").substringBefore(" ("))
                    trimmed.contains("WireGuard configuration written", ignoreCase = true) ->
                        onProgress("Chaves criptográficas geradas com sucesso!")
                    trimmed.contains("Successfully generated config", ignoreCase = true) ->
                        onProgress("Perfil WireGuard finalizado com sucesso!")
                }
                line = reader.readLine()
            }
            val exitCode = process.waitFor()
            val output = outputBuilder.toString()

            if (exitCode == 0 && outputFile.exists() && outputFile.length() > 0) {
                onProgress("Perfil WireGuard ProtonVPN gerado com sucesso!")
                Result.success(outputFile)
            } else {
                val finalMsg = when {
                    output.contains("code 8002") || output.contains("password is not correct", ignoreCase = true) ->
                        "Senha incorreta. Verifique suas credenciais Proton."
                    output.contains("code 9001") || output.contains("human verification", ignoreCase = true) ->
                        "A Proton solicitou verificação humana. Faça login pelo navegador uma vez."
                    output.contains("code 2001") || output.contains("user does not exist", ignoreCase = true) ->
                        "Usuário ou email Proton não encontrado."
                    else -> {
                        val cleanError = output.lines()
                            .map { it.trim() }
                            .filter { it.isNotBlank() && !it.startsWith("Usage:") && !it.startsWith("Flags:") }
                            .lastOrNull { it.startsWith("Error:") }
                        cleanError?.removePrefix("Error: ") ?: "Falha na autenticação (código $exitCode)"
                    }
                }
                Result.failure(Exception(finalMsg))
            }
        } catch (e: Exception) {
            Result.failure(e)
        }
    }
}
