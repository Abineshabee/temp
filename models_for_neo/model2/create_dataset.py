import pandas as pd
import numpy as np
import random
from datetime import datetime, timedelta

def generate_alert_data(num_students=100):
    data = []
    today = datetime.today()
    
    for _ in range(num_students):
        cgpa = round(random.uniform(5.0, 9.5), 2)  # CGPA between 5.0 and 9.5
        gpa = round(random.uniform(5.0, 9.8), 2)  # GPA between 5.0 and 9.8
        attendance = random.randint(50, 100)  # Attendance between 50% and 100%
        assignment_due_days = random.randint(0, 10)  # Assignment due in 0-10 days
        project_due_days = random.randint(0, 20)  # Project due in 0-20 days
        fees_due = random.randint(0, 1)  # Fees due (0: No, 1: Yes)
        
        # Alerts based on conditions
        attendance_alert = 1 if attendance < 70 else 0
        gpa_alert = 1 if gpa < 6.0 else 0
        cgpa_alert = 1 if cgpa < 6.0 else 0
        assignment_alert = 1 if assignment_due_days < 2 else 0
        project_alert = 1 if project_due_days < 7 else 0
        fee_alert = fees_due
        
        data.append([
            cgpa, gpa, attendance, assignment_due_days, project_due_days, fees_due,
            attendance_alert, gpa_alert, cgpa_alert, assignment_alert, project_alert, fee_alert
        ])
    
    columns = [
        "CGPA", "GPA", "Attendance Percentage", "Assignment Due Days", "Project Due Days", "Fees Due",
        "Attendance Alert", "GPA Alert", "CGPA Alert", "Assignment Alert", "Project Alert", "Fee Alert"
    ]
    
    df = pd.DataFrame(data, columns=columns)
    df.to_csv("student_alerts.csv", index=False)
    print("Dataset generated and saved as student_alerts.csv")

# Generate dataset
generate_alert_data(30000) 

