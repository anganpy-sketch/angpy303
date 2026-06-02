//vivecoded stopwatch by @ang
package com.example.stopwatch

import androidx.compose.foundation.layout.*
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.itemsIndexed
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlinx.coroutines.delay

data class Lap(val index: Int, val lapMs: Long, val totalMs: Long)

fun Long.toDisplayTime(): String {
    val h = this / 3_600_000
    val m = (this % 3_600_000) / 60_000
    val s = (this % 60_000) / 1_000
    val cs = (this % 1_000) / 10
    return if (h > 0) "%02d:%02d:%02d.%02d".format(h, m, s, cs)
    else "%02d:%02d.%02d".format(m, s, cs)
}

@Composable
fun StopwatchScreen() {
    var running by remember { mutableStateOf(false) }
    var elapsedMs by remember { mutableLongStateOf(0L) }
    var startTime by remember { mutableLongStateOf(0L) }
    var lastLapMs by remember { mutableLongStateOf(0L) }
    var laps by remember { mutableStateOf(listOf<Lap>()) }

    // Ticker
    LaunchedEffect(running) {
        if (running) {
            startTime = System.currentTimeMillis() - elapsedMs
            while (running) {
                elapsedMs = System.currentTimeMillis() - startTime
                delay(16L) // ~60fps
            }
        }
    }

    val bestLapMs = laps.minOfOrNull { it.lapMs }
    val worstLapMs = if (laps.size > 1) laps.maxOfOrNull { it.lapMs } else null

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(24.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Spacer(Modifier.height(48.dp))

        // Big timer display
        Text(
            text = elapsedMs.toDisplayTime(),
            fontSize = 56.sp,
            fontFamily = FontFamily.Monospace,
            textAlign = TextAlign.Center,
            modifier = Modifier.fillMaxWidth()
        )

        Spacer(Modifier.height(40.dp))

        // Buttons
        Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {
            Button(
                onClick = { running = !running },
                modifier = Modifier.weight(1f)
            ) {
                Text(if (running) "Stop" else "Start")
            }

            OutlinedButton(
                onClick = {
                    val lapTime = elapsedMs - lastLapMs
                    laps = laps + Lap(laps.size + 1, lapTime, elapsedMs)
                    lastLapMs = elapsedMs
                },
                enabled = running,
                modifier = Modifier.weight(1f)
            ) { Text("Lap") }

            OutlinedButton(
                onClick = {
                    running = false
                    elapsedMs = 0L
                    lastLapMs = 0L
                    laps = listOf()
                },
                enabled = elapsedMs > 0,
                modifier = Modifier.weight(1f)
            ) { Text("Reset") }
        }

        Spacer(Modifier.height(24.dp))
        HorizontalDivider()

        // Lap list
        LazyColumn(modifier = Modifier.fillMaxWidth()) {
            itemsIndexed(laps.reversed()) { _, lap ->
                val isBest = lap.lapMs == bestLapMs && laps.size > 1
                val isWorst = lap.lapMs == worstLapMs && laps.size > 1

                val labelColor = when {
                    isBest -> MaterialTheme.colorScheme.primary
                    isWorst -> MaterialTheme.colorScheme.error
                    else -> MaterialTheme.colorScheme.onSurface
                }

                Row(
                    modifier = Modifier
                        .fillMaxWidth()
                        .padding(vertical = 12.dp),
                    horizontalArrangement = Arrangement.SpaceBetween
                ) {
                    Text(
                        "Lap ${lap.index}" + if (isBest) "  ●" else if (isWorst) "  ○" else "",
                        color = labelColor,
                        fontSize = 14.sp
                    )
                    Text(lap.lapMs.toDisplayTime(), fontFamily = FontFamily.Monospace, fontSize = 14.sp)
                    Text(
                        lap.totalMs.toDisplayTime(),
                        fontFamily = FontFamily.Monospace,
                        fontSize = 14.sp,
                        color = MaterialTheme.colorScheme.onSurfaceVariant
                    )
                }
                HorizontalDivider(thickness = 0.5.dp)
            }
        }
    }
}
