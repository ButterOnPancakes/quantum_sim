#include "raylib.h"
#include "../builder/circuit.h"
#include "../simulator/opti_sim.h"
#include "../builder/register.h"
#include "../builder/gaterep.h"
#include "../utils/list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SCREEN_WIDTH 1200
#define SCREEN_HEIGHT 800
#define WIRE_SPACING 60
#define WIRE_START_X 100
#define WIRE_START_Y 150
#define GATE_SIZE 40
#define TOOLBAR_HEIGHT 100

// Tool Enums
typedef enum {
    TOOL_NONE,
    TOOL_H,
    TOOL_X,
    TOOL_Y,
    TOOL_Z,
    TOOL_CNOT,
    TOOL_CUSTOM,
    TOOL_ERASER,
    TOOL_RUN
} ToolType;

// UI State
ToolType currentTool = TOOL_NONE;
int cnotControlQubit = -1; // -1 means no control selected yet

// Custom Gate Menu State
bool showCustomMenu = false;
char customLabel[64] = "U";
char customQubits[64] = "0";
char customMatrix[2048] = "1 0 0 0 0 0 1 0"; // Identity 2x2
int activeTextBox = -1; // 0:Label, 1:Qubits, 2:Matrix

// Button Structure
typedef struct {
    Rectangle rect;
    const char *label;
    ToolType tool;
    Color color;
} ToolButton;

#define NUM_BUTTONS 8
ToolButton buttons[NUM_BUTTONS] = {
    {{20, 20, 60, 60}, "H", TOOL_H, LIGHTGRAY},
    {{100, 20, 60, 60}, "X", TOOL_X, LIGHTGRAY},
    {{180, 20, 60, 60}, "Y", TOOL_Y, LIGHTGRAY},
    {{260, 20, 60, 60}, "Z", TOOL_Z, LIGHTGRAY},
    {{340, 20, 60, 60}, "CNOT", TOOL_CNOT, LIGHTGRAY},
    {{420, 20, 60, 60}, "CUST", TOOL_CUSTOM, LIGHTGRAY},
    {{500, 20, 60, 60}, "DEL", TOOL_ERASER, RED},
    {{580, 20, 60, 60}, "RUN", TOOL_RUN, GREEN}
};

void DrawCircuit(QuantumCircuit *circuit) {
    if (!circuit || !circuit->qregister) return;

    int num_qubits = circuit->qregister->nb_qbits;

    // Draw Qubit Wires
    for (int i = 0; i < num_qubits; i++) {
        int y = WIRE_START_Y + i * WIRE_SPACING;
        DrawText(TextFormat("q[%d]", i), WIRE_START_X - 40, y - 10, 20, BLACK);
        DrawLine(WIRE_START_X, y, SCREEN_WIDTH - 50, y, DARKGRAY);
    }

    // Draw Gates
    struct ListNode *current = circuit->gates->head;
    int gate_step = GATE_SIZE + 20;
    int current_x = WIRE_START_X + 50;

    while (current != NULL) {
        Gate *g = (Gate *)current->data;
        
        if (g->class == UNITARY) {
            int y = WIRE_START_Y + g->gate.unitary.qbit * WIRE_SPACING;
            
            DrawRectangle(current_x, y - GATE_SIZE/2, GATE_SIZE, GATE_SIZE, WHITE);
            DrawRectangleLines(current_x, y - GATE_SIZE/2, GATE_SIZE, GATE_SIZE, BLACK);
            
            const char *gate_label = "?";
            switch(g->gate.unitary.type) {
                case GATE_X: gate_label = "X"; break;
                case GATE_H: gate_label = "H"; break;
                case GATE_Z: gate_label = "Z"; break;
                case GATE_Y: gate_label = "Y"; break;
                default: gate_label = "U"; break;
            }
            int textWidth = MeasureText(gate_label, 20);
            DrawText(gate_label, current_x + (GATE_SIZE - textWidth)/2, y - 10, 20, BLACK);
        }
        else if (g->class == CONTROL) {
             int cy = WIRE_START_Y + g->gate.control.control * WIRE_SPACING;
             int ty = WIRE_START_Y + g->gate.control.qbit * WIRE_SPACING;
             
             DrawCircle(current_x + GATE_SIZE/2, cy, 6, BLACK);
             DrawLine(current_x + GATE_SIZE/2, cy, current_x + GATE_SIZE/2, ty, BLACK);
             
             DrawRectangle(current_x, ty - GATE_SIZE/2, GATE_SIZE, GATE_SIZE, WHITE);
             DrawRectangleLines(current_x, ty - GATE_SIZE/2, GATE_SIZE, GATE_SIZE, BLACK);
             
             const char *target_label = "X"; 
             if (g->gate.control.type == GATE_Z) target_label = "Z"; 

             int textWidth = MeasureText(target_label, 20);
             DrawText(target_label, current_x + (GATE_SIZE - textWidth)/2, ty - 10, 20, BLACK);
        }
        else if (g->class == CUSTOM) {
            for (int i = 0; i < g->gate.custom.nb_qbits; i++) {
                int q = g->gate.custom.qbits[i];
                int y = WIRE_START_Y + q * WIRE_SPACING;
                
                DrawRectangle(current_x, y - GATE_SIZE/2, GATE_SIZE, GATE_SIZE, BEIGE);
                DrawRectangleLines(current_x, y - GATE_SIZE/2, GATE_SIZE, GATE_SIZE, BLACK);
                
                const char *lbl = g->gate.custom.label ? g->gate.custom.label : "C";
                // Simple clipping
                if (strlen(lbl) > 3) {
                     DrawText(TextFormat("%.3s", lbl), current_x + 5, y - 10, 10, BLACK);
                } else {
                     DrawText(lbl, current_x + 5, y - 10, 10, BLACK);
                }
            }
            if (g->gate.custom.nb_qbits > 1) {
                 int min_q = 1000, max_q = -1;
                 for (int i=0; i<g->gate.custom.nb_qbits; i++) {
                     if (g->gate.custom.qbits[i] < min_q) min_q = g->gate.custom.qbits[i];
                     if (g->gate.custom.qbits[i] > max_q) max_q = g->gate.custom.qbits[i];
                 }
                 int y1 = WIRE_START_Y + min_q * WIRE_SPACING;
                 int y2 = WIRE_START_Y + max_q * WIRE_SPACING;
                 DrawLine(current_x + GATE_SIZE/2, y1, current_x + GATE_SIZE/2, y2, BLACK);
            }
        }

        current_x += gate_step;
        current = current->next;
    }
}

void DrawToolbar() {
    DrawRectangle(0, 0, SCREEN_WIDTH, TOOLBAR_HEIGHT, RAYWHITE);
    DrawLine(0, TOOLBAR_HEIGHT, SCREEN_WIDTH, TOOLBAR_HEIGHT, GRAY);

    for (int i = 0; i < NUM_BUTTONS; i++) {
        Color btnColor = buttons[i].color;
        if (currentTool == buttons[i].tool) {
            btnColor = SKYBLUE; 
        }
        
        DrawRectangleRec(buttons[i].rect, btnColor);
        DrawRectangleLinesEx(buttons[i].rect, 2, DARKGRAY);
        
        int textWidth = MeasureText(buttons[i].label, 20);
        DrawText(buttons[i].label, 
                 buttons[i].rect.x + (buttons[i].rect.width - textWidth)/2, 
                 buttons[i].rect.y + (buttons[i].rect.height - 20)/2, 
                 20, BLACK);
    }

    DrawText("Select a tool. 'CUST' opens Custom Gate Menu.", 600, 30, 20, DARKGRAY);
    if (currentTool == TOOL_CNOT && cnotControlQubit != -1) {
        DrawText(TextFormat("Select Target for CNOT (Control: q[%d])", cnotControlQubit), 600, 60, 20, MAROON);
    }
}

void ParseAndAddCustomGate(QuantumCircuit *qc) {
    // Parse Qubits
    int qubits[100]; 
    int q_count = 0;
    
    char qBuf[64];
    strcpy(qBuf, customQubits);
    char *token = strtok(qBuf, ", ");
    while (token != NULL) {
        if (q_count < 100) {
            qubits[q_count++] = atoi(token);
        }
        token = strtok(NULL, ", ");
    }
    
    if (q_count == 0) return; 
    
    long required_entries = 1;
    for(int i=0; i<q_count; i++) required_entries *= 4;
    
    double complex *mat = malloc(required_entries * sizeof(double complex));
    
    char mBuf[2048];
    strncpy(mBuf, customMatrix, 2047);
    mBuf[2047] = '\0';
    
    token = strtok(mBuf, " ,;\n"); 
    double re = 0, im = 0;
    
    for (int i=0; i < required_entries; i++) {
        if (token) { re = atof(token); token = strtok(NULL, " ,;\n"); } else re = 0;
        if (token) { im = atof(token); token = strtok(NULL, " ,;\n"); } else im = 0;
        mat[i] = re + im * I;
    }
    
    int *final_qubits = malloc(q_count * sizeof(int));
    for(int i=0; i<q_count; i++) final_qubits[i] = qubits[i];
    
    char *final_label = malloc(strlen(customLabel) + 1);
    strcpy(final_label, customLabel);
    
    add_custom_gate(qc, q_count, final_qubits, mat, final_label);
}

void DrawCustomMenu(QuantumCircuit *qc) {
    if (!showCustomMenu) return; 
    
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));
    
    int winW = 600;
    int winH = 500;
    int winX = (SCREEN_WIDTH - winW) / 2;
    int winY = (SCREEN_HEIGHT - winH) / 2;
    
    DrawRectangle(winX, winY, winW, winH, RAYWHITE);
    DrawRectangleLines(winX, winY, winW, winH, DARKGRAY);
    DrawText("Add Custom Gate", winX + 20, winY + 20, 20, BLACK);
    
    // Label
    DrawText("Label:", winX + 20, winY + 60, 20, DARKGRAY);
    Rectangle labelBox = {winX + 100, winY + 60, 200, 30};
    DrawRectangleRec(labelBox, activeTextBox == 0 ? LIGHTGRAY : WHITE);
    DrawRectangleLinesEx(labelBox, 1, activeTextBox == 0 ? BLUE : DARKGRAY);
    DrawText(customLabel, labelBox.x + 5, labelBox.y + 5, 20, BLACK);
    
    // Qubits
    DrawText("Qubits:", winX + 20, winY + 110, 20, DARKGRAY);
    Rectangle qubitsBox = {winX + 100, winY + 110, 400, 30};
    DrawRectangleRec(qubitsBox, activeTextBox == 1 ? LIGHTGRAY : WHITE);
    DrawRectangleLinesEx(qubitsBox, 1, activeTextBox == 1 ? BLUE : DARKGRAY);
    DrawText(customQubits, qubitsBox.x + 5, qubitsBox.y + 5, 20, BLACK);
    DrawText("Index list (e.g. 0,1)", winX + 510, winY + 115, 10, GRAY);
    
    // Matrix
    DrawText("Matrix:", winX + 20, winY + 160, 20, DARKGRAY);
    DrawText("Format: Re Im Re Im ... (Row-major)", winX + 100, winY + 160, 10, GRAY);
    Rectangle matBox = {winX + 20, winY + 190, 560, 240};
    DrawRectangleRec(matBox, activeTextBox == 2 ? LIGHTGRAY : WHITE);
    DrawRectangleLinesEx(matBox, 1, activeTextBox == 2 ? BLUE : DARKGRAY);
    
    // Multi-line text display (simple)
    DrawText(customMatrix, matBox.x + 5, matBox.y + 5, 10, BLACK);
    
    // Buttons
    Rectangle closeBtn = {winX + winW - 40, winY + 10, 30, 30};
    DrawRectangleRec(closeBtn, RED);
    DrawText("X", closeBtn.x + 10, closeBtn.y + 5, 20, WHITE);
    
    Rectangle addBtn = {winX + winW/2 - 50, winY + winH - 60, 100, 40};
    DrawRectangleRec(addBtn, GREEN);
    DrawText("ADD", addBtn.x + 30, addBtn.y + 10, 20, WHITE);
}

bool showResults = false;

void DrawResults(QuantumCircuit *qc) {
    if (!showResults) return;

    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.5f));

    int winW = 800;
    int winH = 600;
    int winX = (SCREEN_WIDTH - winW) / 2;
    int winY = (SCREEN_HEIGHT - winH) / 2;

    DrawRectangle(winX, winY, winW, winH, RAYWHITE);
    DrawRectangleLines(winX, winY, winW, winH, DARKGRAY);
    DrawText("Simulation Results", winX + 20, winY + 20, 20, BLACK);

    Rectangle closeBtn = {winX + winW - 40, winY + 10, 30, 30};
    DrawRectangleRec(closeBtn, RED);
    DrawText("X", closeBtn.x + 10, closeBtn.y + 5, 20, WHITE);

    if (CheckCollisionPointRec(GetMousePosition(), closeBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        showResults = false;
    }

    int num_states = 1 << qc->qregister->nb_qbits;
    int drawn_count = 0;
    int start_y = winY + 60;
    int col_x = winX + 40;

    for (int i = 0; i < num_states; i++) {
        double complex amp = qc->qregister->statevector[i];
        double prob = creal(amp)*creal(amp) + cimag(amp)*cimag(amp);

        if (prob > 0.0001) {
            char bin_str[32];
            for(int b=0; b<qc->qregister->nb_qbits; b++) {
                 bin_str[qc->qregister->nb_qbits - 1 - b] = ((i >> b) & 1) ? '1' : '0';
            }
            bin_str[qc->qregister->nb_qbits] = '\0';

            DrawText(TextFormat("|%s>: %.4f", bin_str, prob), col_x, start_y + drawn_count * 25, 20, BLACK);
            
            DrawRectangle(col_x + 150, start_y + drawn_count * 25, (int)(prob * 400), 20, BLUE);
            
            drawn_count++;
            if (start_y + drawn_count * 25 > winY + winH - 40) {
                 col_x += 350;
                 drawn_count = 0;
                 if (col_x > winX + winW - 100) break; 
            }
        }
    }
}

void HandleCustomMenuInput(QuantumCircuit *qc) {
    int winW = 600;
    int winH = 500;
    int winX = (SCREEN_WIDTH - winW) / 2;
    int winY = (SCREEN_HEIGHT - winH) / 2;
    
    Rectangle labelBox = {winX + 100, winY + 60, 200, 30};
    Rectangle qubitsBox = {winX + 100, winY + 110, 400, 30};
    Rectangle matBox = {winX + 20, winY + 190, 560, 240};
    Rectangle closeBtn = {winX + winW - 40, winY + 10, 30, 30};
    Rectangle addBtn = {winX + winW/2 - 50, winY + winH - 60, 100, 40};
    
    Vector2 m = GetMousePosition();
    
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckCollisionPointRec(m, labelBox)) activeTextBox = 0;
        else if (CheckCollisionPointRec(m, qubitsBox)) activeTextBox = 1;
        else if (CheckCollisionPointRec(m, matBox)) activeTextBox = 2;
        else activeTextBox = -1;
        
        if (CheckCollisionPointRec(m, closeBtn)) showCustomMenu = false;
        if (CheckCollisionPointRec(m, addBtn)) {
            ParseAndAddCustomGate(qc);
            showCustomMenu = false;
        }
    }
    
    // Text Entry
    if (activeTextBox != -1) {
        char *buffer = NULL;
        int maxLen = 0;
        if (activeTextBox == 0) { buffer = customLabel; maxLen = 63; }
        else if (activeTextBox == 1) { buffer = customQubits; maxLen = 63; }
        else if (activeTextBox == 2) { buffer = customMatrix; maxLen = 2047; }
        
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125)) {
                int len = strlen(buffer);
                if (len < maxLen) {
                    buffer[len] = (char)key;
                    buffer[len+1] = '\0';
                }
            }
            key = GetCharPressed();
        }
        
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = strlen(buffer);
            if (len > 0) buffer[len-1] = '\0';
        }
    }
}

void HandleInput(QuantumCircuit *qc) {
    if (showResults) return;

    if (showCustomMenu) {
        HandleCustomMenuInput(qc);
        return;
    }

    Vector2 mousePos = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (mousePos.y < TOOLBAR_HEIGHT) {
            for (int i = 0; i < NUM_BUTTONS; i++) {
                if (CheckCollisionPointRec(mousePos, buttons[i].rect)) {
                    if (buttons[i].tool == TOOL_CUSTOM) {
                        showCustomMenu = true;
                    } else if (buttons[i].tool == TOOL_RUN) {
                         int dim = 1 << qc->qregister->nb_qbits;
                         for (int k = 0; k < dim; k++) qc->qregister->statevector[k] = 0;
                         qc->qregister->statevector[0] = 1;
                         
                         circuit_execute(qc, false);
                         showResults = true;
                         currentTool = TOOL_NONE;
                         return;
                    }
                    currentTool = buttons[i].tool;
                    cnotControlQubit = -1; 
                    return;
                }
            }
        } 
        else {
            if (currentTool == TOOL_ERASER) {
                 ListIterator iter = list_iterator_begin(qc->gates);
                 int gate_step = GATE_SIZE + 20;
                 int current_x = WIRE_START_X + 50;
                 
                 while (iter.current != NULL) {
                     Gate *g = (Gate *)iter.current->data;
                     bool hit = false;
                     Rectangle hitBox = {0};
                     
                     if (g->class == UNITARY) {
                         int y = WIRE_START_Y + g->gate.unitary.qbit * WIRE_SPACING;
                         hitBox = (Rectangle){current_x, y - GATE_SIZE/2, GATE_SIZE, GATE_SIZE};
                         if (CheckCollisionPointRec(mousePos, hitBox)) hit = true;
                     }
                     else if (g->class == CONTROL) {
                         int ty = WIRE_START_Y + g->gate.control.qbit * WIRE_SPACING;
                         int cy = WIRE_START_Y + g->gate.control.control * WIRE_SPACING;
                         hitBox = (Rectangle){current_x, ty - GATE_SIZE/2, GATE_SIZE, GATE_SIZE};
                         if (CheckCollisionPointRec(mousePos, hitBox)) hit = true;
                         Rectangle cBox = {current_x + GATE_SIZE/2 - 10, cy - 10, 20, 20};
                         if (CheckCollisionPointRec(mousePos, cBox)) hit = true;
                     }
                     else if (g->class == CUSTOM) {
                         for (int i=0; i<g->gate.custom.nb_qbits; i++) {
                             int q = g->gate.custom.qbits[i];
                             int y = WIRE_START_Y + q * WIRE_SPACING;
                             hitBox = (Rectangle){current_x, y - GATE_SIZE/2, GATE_SIZE, GATE_SIZE};
                             if (CheckCollisionPointRec(mousePos, hitBox)) { hit = true; break; }
                         }
                     }

                     if (hit) {
                         if (g->class == CUSTOM) {
                             free(g->gate.custom.qbits);
                             free(g->gate.custom.mat);
                             free(g->gate.custom.label);
                         }
                         free(g);
                         list_iterator_remove_current(&iter);
                         break;
                     }

                     current_x += gate_step;
                     list_iterator_next(&iter);
                 }
                 return;
            }

            int num_qubits = qc->qregister->nb_qbits;
            int clickedQubit = -1;

            for (int i = 0; i < num_qubits; i++) {
                int y = WIRE_START_Y + i * WIRE_SPACING;
                if (mousePos.y >= y - 20 && mousePos.y <= y + 20) {
                    clickedQubit = i;
                    break;
                }
            }

            if (clickedQubit != -1 && currentTool != TOOL_NONE) {
                switch (currentTool) {
                    case TOOL_H:
                        add_unitary_gate(qc, clickedQubit, GATE_H, 0);
                        break;
                    case TOOL_X:
                        add_unitary_gate(qc, clickedQubit, GATE_X, 0);
                        break;
                    case TOOL_Y:
                        add_unitary_gate(qc, clickedQubit, GATE_Y, 0);
                        break;
                    case TOOL_Z:
                        add_unitary_gate(qc, clickedQubit, GATE_Z, 0);
                        break;
                    case TOOL_CNOT:
                        if (cnotControlQubit == -1) {
                            cnotControlQubit = clickedQubit;
                        } else {
                            if (cnotControlQubit != clickedQubit) {
                                add_control_gate(qc, cnotControlQubit, clickedQubit, GATE_X, 0);
                                cnotControlQubit = -1;
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
}

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Quantum Circuit Builder");
    SetTargetFPS(60);

    QuantumRegister *qreg = qregister_create(5);
    ClassicalRegister *creg = cregister_create(5);
    QuantumCircuit *qc = circuit_create(qreg, creg);

    while (!WindowShouldClose()) {
        HandleInput(qc);

        BeginDrawing();
            ClearBackground(WHITE);
            
            DrawToolbar();
            DrawCircuit(qc);
            
            if (currentTool == TOOL_CNOT && cnotControlQubit != -1) {
                 int cy = WIRE_START_Y + cnotControlQubit * WIRE_SPACING;
                 Vector2 m = GetMousePosition();
                 DrawCircle(WIRE_START_X + 20, cy, 5, MAROON);
                 DrawLine(WIRE_START_X + 20, cy, m.x, m.y, MAROON); 
            }

            if (!showCustomMenu && currentTool != TOOL_NONE && GetMousePosition().y > TOOLBAR_HEIGHT) {
                Vector2 m = GetMousePosition();
                Color ghostColor = Fade(LIGHTGRAY, 0.5f);
                if (currentTool == TOOL_ERASER) ghostColor = Fade(RED, 0.5f);

                DrawRectangle(m.x - GATE_SIZE/2, m.y - GATE_SIZE/2, GATE_SIZE, GATE_SIZE, ghostColor);
                DrawRectangleLines(m.x - GATE_SIZE/2, m.y - GATE_SIZE/2, GATE_SIZE, GATE_SIZE, GRAY);
                
                const char *label = "";
                switch (currentTool) {
                    case TOOL_H: label = "H"; break;
                    case TOOL_X: label = "X"; break;
                    case TOOL_Y: label = "Y"; break;
                    case TOOL_Z: label = "Z"; break;
                    case TOOL_CNOT: label = (cnotControlQubit == -1) ? "C" : "T"; break;
                    case TOOL_CUSTOM: label = "CS"; break;
                    case TOOL_ERASER: label = "DEL"; break;
                    default: break;
                }
                DrawText(label, m.x - 10, m.y - 10, 20, DARKGRAY);
            }
            
            DrawCustomMenu(qc);
            DrawResults(qc);

        EndDrawing();
    }

    // circuit_free(qc); 
    CloseWindow();

    return 0;
}
