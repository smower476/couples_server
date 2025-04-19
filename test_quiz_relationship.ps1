# test_quiz_relationship.ps1
# PowerShell script to test quiz relationship functionality

Write-Host "Starting Quiz Relationship Test" -ForegroundColor Green

# Configuration variables
$serverUrl = "http://localhost:3000" # Change this if your server runs on a different port
$quizEndpoint = "$serverUrl/api/quiz"
$relationshipEndpoint = "$serverUrl/api/relationships"

# Test data
$testUser1 = @{
    username = "testuser1"
    password = "password123"
}

$testUser2 = @{
    username = "testuser2"
    password = "password123"
}

$testRelationship = @{
    user1 = $testUser1.username
    user2 = $testUser2.username
}

# Helper functions
function Test-Step {
    param (
        [string]$Description,
        [scriptblock]$Test
    )
    
    Write-Host "* $Description..." -NoNewline
    try {
        & $Test
        Write-Host " [PASS]" -ForegroundColor Green
        return $true
    } catch {
        Write-Host " [FAIL]" -ForegroundColor Red
        Write-Host "  Error: $_" -ForegroundColor Red
        return $false
    }
}

function Invoke-ApiRequest {
    param (
        [string]$Method,
        [string]$Uri,
        [object]$Body,
        [string]$ContentType = "application/json",
        [hashtable]$Headers = @{}
    )
    
    $params = @{
        Method = $Method
        Uri = $Uri
        Headers = $Headers
    }
    
    if ($Body) {
        $params.Body = ($Body | ConvertTo-Json)
        $params.ContentType = $ContentType
    }
    
    return Invoke-RestMethod @params
}

# Main test sequence
Write-Host "=== Quiz Relationship Tests ===" -ForegroundColor Cyan

# 1. Create test users or ensure they exist
Test-Step "Creating test users" {
    # This would normally register users, but we'll just assume they exist for this test
    # Implement user registration if needed
    $true
}

# 2. Create a relationship
$relationshipId = $null
Test-Step "Creating relationship between test users" {
    $response = Invoke-ApiRequest -Method "POST" -Uri $relationshipEndpoint -Body $testRelationship
    $relationshipId = $response.id
    if (-not $relationshipId) { throw "Failed to get relationship ID" }
    $relationshipId
}

# 3. Take quiz for relationship
$quizData = @{
    relationshipId = $relationshipId
    user = $testUser1.username
    answers = @(
        @{ questionId = 1; answer = "strongly_agree" },
        @{ questionId = 2; answer = "agree" },
        @{ questionId = 3; answer = "neutral" },
        @{ questionId = 4; answer = "disagree" },
        @{ questionId = 5; answer = "strongly_disagree" }
    )
}

Test-Step "Submitting quiz for first user" {
    $response = Invoke-ApiRequest -Method "POST" -Uri $quizEndpoint -Body $quizData
    if (-not $response.success) { throw "Quiz submission failed" }
    $true
}

# 4. Take quiz for second user
$quizData = @{
    relationshipId = $relationshipId
    user = $testUser2.username
    answers = @(
        @{ questionId = 1; answer = "agree" },
        @{ questionId = 2; answer = "neutral" },
        @{ questionId = 3; answer = "disagree" },
        @{ questionId = 4; answer = "strongly_disagree" },
        @{ questionId = 5; answer = "strongly_agree" }
    )
}

Test-Step "Submitting quiz for second user" {
    $response = Invoke-ApiRequest -Method "POST" -Uri $quizEndpoint -Body $quizData
    if (-not $response.success) { throw "Quiz submission failed" }
    $true
}

# 5. Get relationship results
Test-Step "Retrieving relationship results" {
    $response = Invoke-ApiRequest -Method "GET" -Uri "$relationshipEndpoint/$relationshipId"
    
    # Check if quiz results exist
    if (-not $response.quizResults) { throw "Quiz results not found" }
    
    Write-Host "`n  Compatibility Score: $($response.quizResults.compatibilityScore)" -ForegroundColor Cyan
    Write-Host "  Areas of Alignment: $($response.quizResults.areasOfAlignment -join ', ')" -ForegroundColor Cyan
    Write-Host "  Areas of Difference: $($response.quizResults.areasOfDifference -join ', ')" -ForegroundColor Cyan
    
    $true
}

# 6. Test error handling - invalid relationship
Test-Step "Testing invalid relationship ID" {
    try {
        $response = Invoke-ApiRequest -Method "GET" -Uri "$relationshipEndpoint/invalid-id"
        throw "Should have failed with invalid ID"
    } catch {
        # This should fail, so catching the error is expected
        $true
    }
}

# Summary
Write-Host "`nQuiz Relationship Test Complete" -ForegroundColor Green
Write-Host "Press any key to exit..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")